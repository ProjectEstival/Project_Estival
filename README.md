# **Project_Estival**  
## SAE ATHENS LABS STUDIO PROJ 26S2  

> ### Before you do anything to the project make **sure**, you understand the **Git workflow** completely. </br>
### **https://app.clickup.com/90151230104/v/dc/2kypx6mr-6075/2kypx6mr-19335**  
# <span style="color: purple;"> **When writing code follow the architecture below**  </span>  
# **SOLID**  
### **S** – Single Responsibility Principle (SRP)
> *A class should have one, and only one, reason to change.*

* **In Unreal:** Avoid creating massive `AActor` or `UObject` classes that handle movement, health, inventory, and network replication all in one place.
* **How to Apply:** Break systems into specialized `UActorComponent` classes.
  * <span style="color:#e06c75;">**Bad:**</span> `AMainCharacter` handles health calculations, inventory management, and weapon shooting.
  * <span style="color:#98c379;">**Good:**</span> `AMainCharacter` aggregates a `UHealthComponent`, `UInventoryComponent`, and `UCombatComponent`.

---

### **O** – Open/Closed Principle (OCP)
> *Software entities should be open for extension, but closed for modification.*

* **In Unreal:** Add new gameplay features by creating derived classes or extensions rather than adding massive `switch` statements or conditional checks inside existing classes.
* **How to Apply:** Rely on virtual functions or polymorphic event structures.
  * <span style="color:#e06c75;">**Bad:**</span> A `TakeDamage` function with an `if/else` block checking for every damage type (`Fire`, `Ice`, `Poison`).
  * <span style="color:#98c379;">**Good:**</span> Create a base `UDamageType` or `UDamageEffect` base class and override `ApplyDamage()` in sub-classes (`UFireDamageEffect`, `UIceDamageEffect`).

---

### **L** – Liskov Substitution Principle (LSP)
> *Subtypes must be substitutable for their base types without altering program correctness.*

* **In Unreal:** Derived classes must fulfill the contract of their base class without throwing unexpected errors, null pointers, or ignoring base behavior.
* **How to Apply:** Ensure derived components or actors can be swapped in seamlessly.
  * <span style="color:#e06c75;">**Bad:**</span> A `AFlyingEnemy` subclass that overrides `MoveToTarget()` but leaves it empty or breaks when grounded logic is executed.
  * <span style="color:#98c379;">**Good:**</span> Restructure movement capabilities using interfaces or components so derived actors don't break expected base implementations.

---

### **I** – Interface Segregation Principle (ISP)
> *Clients should not be forced to depend upon interfaces they do not use.*

* **In Unreal:** Keep `UInterface` declarations focused and lean. Do not lump unrelated methods into a single monolithic interface.
* **How to Apply:** Create smaller, specific `UInterface` types.
  * <span style="color:#e06c75;">**Bad:**</span> A single `IInteractableInterface` that requires implementing `Interact()`, `OpenDoor()`, `PickUp()`, and `Inspect()`.
  * <span style="color:#98c379;">**Good:**</span> Split into targeted interfaces like `IInteractable`, `IPickupable`, and `IInspectable`.

---

### **D** – Dependency Inversion Principle (DIP)
> *High-level modules should not depend on low-level modules. Both should depend on abstractions.*

* **In Unreal:** Avoid hard-coding direct `Cast<AMySpecificActor>` references to low-level classes throughout high-level gameplay code.
* **How to Apply:** Reference components or `UInterface` types instead of concrete class types.
  * <span style="color:#e06c75;">**Bad:**</span> A `ATrap` actor directly casting to `AMainPlayerCharacter` to apply damage.
  * <span style="color:#98c379;">**Good:**</span> The `ATrap` queries if the overlapping actor implements `IDamageableInterface` or holds a `UHealthComponent`.