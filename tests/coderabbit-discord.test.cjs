// Run with: node --test tests/coderabbit-discord.test.cjs
// Execute the actual actions/github-script body with isolated API doubles.
const assert = require('node:assert/strict');
const { readFileSync } = require('node:fs');
const { test } = require('node:test');
const { runInNewContext } = require('node:vm');

const workflow = readFileSync(
  new URL('../.github/workflows/coderabbit-discord.yml', `file://${__filename}`),
  'utf8',
);
const scriptMatch = workflow.match(/^([ \t]*)script: \|\r?\n((?:\1[ \t]+[^\n]*(?:\n|$)|\s*\n)+)/m);
assert.ok(scriptMatch, 'The workflow must contain an inline github-script body');
const script = scriptMatch[2].replace(new RegExp(`^${scriptMatch[1]}  `, 'gm'), '');
const sha = '0123456789abcdef0123456789abcdef01234567';
const bot = 'coderabbitai[bot]';
const pr = {
  number: 90, state: 'open', title: 'Review notifications',
  html_url: 'https://github.com/example/project/pull/90',
  head: { sha, ref: 'feature' }, base: { ref: 'main' },
  user: { login: 'contributor', avatar_url: 'https://example.com/avatar.png' },
};
const comment = (body, extra = {}) => ({ user: { login: bot }, body, ...extra });
const review = (body, extra = {}) => comment(body, { commit_id: sha, ...extra });
const finding = (body, extra = {}) => review(body, {
  html_url: 'https://github.com/example/project/pull/90#discussion_r1', ...extra,
});

async function notify(options = {}) {
  const requests = [];
  const pagination = [];
  const lookups = [];
  const messages = [];
  const failures = [];
  const routes = { comments: 'comments', reviews: 'reviews', inline: 'inline' };
  const github = {
    rest: {
      repos: { listPullRequestsAssociatedWithCommit: async args => {
        lookups.push(args);
        if (options.lookupError) throw options.lookupError;
        return { data: options.prs ?? [pr] };
      } },
      issues: { listComments: routes.comments },
      pulls: { listReviews: routes.reviews, listReviewComments: routes.inline },
    },
    paginate: async (route, args) => {
      pagination.push({ route, args });
      if (options.paginationError) throw options.paginationError;
      return options[route] ?? [];
    },
  };
  await runInNewContext(`(async () => {\n${script}\n})()`, {
    github,
    context: { repo: { owner: 'example', repo: 'project' }, payload: { sha } },
    core: { info: text => messages.push(text), setFailed: text => failures.push(text) },
    process: { env: { DISCORD_WEBHOOK_URL: 'https://example.invalid/synthetic-webhook' } },
    fetch: async (url, init) => {
      requests.push({ url, ...init, payload: JSON.parse(init.body) });
      if (options.fetchError) throw options.fetchError;
      return options.response ?? { ok: true };
    },
  }, { timeout: 1000 });
  return { requests, pagination, lookups, messages, failures };
}
const embed = result => result.requests[0].payload.embeds[0];
const field = (result, name) => embed(result).fields.find(item => item.name === name)?.value;

// Verify the workflow's entry condition separately from its inline script.
test('only successful completed CodeRabbit statuses trigger notifications', () => {
  const condition = workflow.match(/^    if: >-\n([\s\S]*?)\n    runs-on:/m)?.[1].trim();
  assert.ok(condition);
  const event = { context: 'CodeRabbit', state: 'success', description: 'Review completed' };
  const cases = [
    [event, true],
    [{ ...event, context: 'other-check' }, false],
    [{ ...event, state: 'pending' }, false],
    [{ ...event, state: 'failure' }, false],
    [{ ...event, description: 'Review in progress' }, false],
  ];
  for (const [status, expected] of cases) {
    assert.equal(runInNewContext(condition, { github: { event: status } }), expected);
  }
});

test('sends one correctly addressed Discord embed with safe mentions and defaults', async () => {
  const result = await notify();
  assert.equal(result.requests.length, 1);
  const request = result.requests[0];
  assert.equal(request.url, 'https://example.invalid/synthetic-webhook');
  assert.equal(request.method, 'POST');
  assert.equal(request.headers['Content-Type'], 'application/json');
  assert.deepEqual(request.payload.allowed_mentions, { parse: [] });
  assert.equal(request.payload.username, 'CodeRabbit');
  assert.equal(request.payload.embeds.length, 1);
  assert.deepEqual(embed(result), {
    title: '🐰 PR #90: Review notifications', url: pr.html_url,
    description: 'CodeRabbit finished reviewing this PR.', color: 0x2ECC71,
    author: { name: 'contributor', icon_url: pr.user.avatar_url },
    fields: [
      { name: 'Actionable', value: '0', inline: true },
      { name: 'Nitpicks', value: '0', inline: true },
      { name: 'Branch', value: 'feature → main', inline: false },
    ], footer: { text: 'Commit 0123456' },
  });
  assert.deepEqual(result.failures, []);
  assert.deepEqual(result.messages, []);
});

test('queries the event commit and paginates all three review sources', async () => {
  const result = await notify();
  // Convert values created in the VM to this realm before strict comparisons.
  assert.deepEqual(JSON.parse(JSON.stringify(result.lookups)), [
    { owner: 'example', repo: 'project', commit_sha: sha },
  ]);
  assert.deepEqual(JSON.parse(JSON.stringify(result.pagination)), [
    { route: 'comments', args: { owner: 'example', repo: 'project', issue_number: 90, per_page: 100 } },
    { route: 'reviews', args: { owner: 'example', repo: 'project', pull_number: 90, per_page: 100 } },
    { route: 'inline', args: { owner: 'example', repo: 'project', pull_number: 90, per_page: 100 } },
  ]);
});

for (const [label, prs] of [
  ['no associated PR', []],
  ['closed PR', [{ ...pr, state: 'closed' }]],
  ['stale commit', [{ ...pr, head: { ...pr.head, sha: 'older-commit' } }]],
]) {
  test(`skips notification and pagination for ${label}`, async () => {
    const result = await notify({ prs });
    assert.equal(result.requests.length, 0);
    assert.equal(result.pagination.length, 0);
    assert.equal(result.messages.length, 1);
    assert.match(result.messages[0], /No open PR/);
    assert.deepEqual(result.failures, []);
  });
}

test('selects the open PR at the event SHA among unrelated associations', async () => {
  const result = await notify({ prs: [
    { ...pr, state: 'closed', number: 1 },
    { ...pr, head: { ...pr.head, sha: 'old' }, number: 2 }, pr,
  ] });
  assert.equal(embed(result).url, pr.html_url);
  assert.ok(result.pagination.every(item => (item.args.pull_number ?? item.args.issue_number) === 90));
});

for (const ending of ['\n## Changes\nExcluded', '\n</details>\nExcluded', '']) {
  test(`extracts and trims the bot walkthrough up to ${ending || 'end of comment'}`, async () => {
    const result = await notify({ comments: [
      comment('walkthrough_start\n## Walkthrough\nWrong author', { user: { login: 'human' } }),
      comment('## Walkthrough\nMissing marker'),
      comment(`walkthrough_start\n## Walkthrough\n  Review summary  ${ending}`),
    ] });
    assert.equal(embed(result).description, 'Review summary');
  });
}

test('uses a fallback when the marked walkthrough has no summary heading', async () => {
  const result = await notify({ comments: [comment('walkthrough_start\nNo heading')] });
  assert.equal(embed(result).description, 'CodeRabbit finished reviewing this PR.');
});

test('counts only bot reviews for the event revision and changes warning color', async () => {
  const result = await notify({ reviews: [
    review('Actionable comments posted: 99\nNitpick comments (88)', { user: { login: 'human' } }),
    review('Actionable comments posted: 77', { commit_id: 'old' }),
    review('Actionable comments posted: 12\nNitpick comments (3)'),
  ] });
  assert.equal(field(result, 'Actionable'), '12');
  assert.equal(field(result, 'Nitpicks'), '3');
  assert.equal(embed(result).color, 0xE67E22);
});

test('zero actionable findings remain green even with nitpicks', async () => {
  const result = await notify({ reviews: [review('Actionable comments posted: 0\nNitpick comments (4)')] });
  assert.equal(field(result, 'Nitpicks'), '4');
  assert.equal(embed(result).color, 0x2ECC71);
});

test('includes only current top-level bot findings, removes brackets, and handles missing titles', async () => {
  const result = await notify({ inline: [
    finding('**Ignored human**', { user: { login: 'human' } }),
    finding('**Ignored old**', { commit_id: 'old' }),
    finding('**Ignored reply**', { in_reply_to_id: 123 }),
    finding('Details: **Fix [unsafe] brackets**'),
    finding('Plain comment', { html_url: 'https://example.com/finding2' }),
  ] });
  assert.equal(field(result, 'Findings'),
    `• [Fix unsafe brackets](${pr.html_url}#discussion_r1)\n• [Review comment](https://example.com/finding2)`);
});

test('clips long finding titles to 90 characters with an ellipsis', async () => {
  const result = await notify({ inline: [finding(`**${'x'.repeat(91)}**`)] });
  assert.equal(field(result, 'Findings'), `• [${'x'.repeat(89)}…](${pr.html_url}#discussion_r1)`);
});

test('bounds findings to 1024 characters and reports every omitted finding', async () => {
  const inline = Array.from({ length: 30 }, (_, i) => finding(`**Finding ${i} ${'x'.repeat(80)}**`));
  const result = await notify({ inline });
  const list = field(result, 'Findings');
  const shown = list.split('\n').filter(line => line.startsWith('• ')).length;
  assert.ok(shown > 0 && shown < inline.length);
  assert.ok(list.length <= 1024);
  assert.ok(list.endsWith(`…and ${inline.length - shown} more on the PR`));
  assert.ok(list.includes('Finding 0'));
});

test('reports overflow when even the first finding cannot fit', async () => {
  const result = await notify({ inline: [finding('**Long URL**', { html_url: `https://example.com/${'x'.repeat(1024)}` })] });
  assert.equal(field(result, 'Findings'), '…and 1 more on the PR');
});

for (const excess of [0, 1]) {
  test(`preserves exact limits and clips overflow of ${excess} characters`, async () => {
    const prefix = '🐰 PR #90: ';
    const branchSuffix = ' → main';
    const title = 't'.repeat(256 - prefix.length + excess);
    const summary = 's'.repeat(3500 + excess);
    const branch = 'b'.repeat(1024 - branchSuffix.length + excess);
    const result = await notify({
      prs: [{ ...pr, title, head: { ...pr.head, ref: branch } }],
      comments: [comment(`walkthrough_start\n## Walkthrough\n${summary}`)],
    });
    assert.equal(embed(result).title, excess ? `${(prefix + title).slice(0, 255)}…` : prefix + title);
    assert.equal(embed(result).description, excess ? `${'s'.repeat(3499)}…` : summary);
    assert.equal(field(result, 'Branch'), excess ? `${(branch + branchSuffix).slice(0, 1023)}…` : branch + branchSuffix);
  });
}

test('marks unsuccessful Discord delivery as failed with status and response', async () => {
  const result = await notify({ response: { ok: false, status: 429, text: async () => 'Rate limited' } });
  assert.deepEqual(result.failures, ['Discord returned 429: Rate limited']);
  assert.equal(result.requests.length, 1);
});

for (const source of ['lookupError', 'paginationError', 'fetchError']) {
  test(`propagates ${source} so the action fails rather than reporting success`, async () => {
    const error = new Error(`${source} unavailable`);
    await assert.rejects(notify({ [source]: error }), candidate => candidate === error);
  });
}
