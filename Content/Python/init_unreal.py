"""Auto-runs at editor startup. Registers the Character Swapper Tools menu.

All the actual logic lives in character_swapper.py -- this file only exists so
Unreal registers the menu without you having to do anything.

TO USE IN ANOTHER PROJECT: copy both files into <Project>/Content/Python/.
That is the whole installation.
"""

import character_swapper

character_swapper.register_menu()
