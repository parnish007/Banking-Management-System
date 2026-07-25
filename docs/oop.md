# Object-Oriented Design in the Banking Management System

This document explains how the Banking Management System demonstrates object-oriented programming concepts and which design principles the project follows.

## Table of Contents

1. [OOP Overview](#oop-overview)
2. [How the project expresses OOP principles](#how-the-project-expresses-oop-principles)
   1. [Abstraction](#abstraction)
   2. [Encapsulation](#encapsulation)
   3. [Inheritance](#inheritance)
   4. [Polymorphism](#polymorphism)
3. [Core project objects and modules](#core-project-objects-and-modules)
   1. [Data objects](#data-objects)
   2. [Service and behavior modules](#service-and-behavior-modules)
4. [Design principles followed](#design-principles-followed)
   1. [Single Responsibility Principle (SRP)](#single-responsibility-principle-srp)
   2. [Separation of Concerns](#separation-of-concerns)
   3. [Dependency Direction / Layering](#dependency-direction--layering)
   4. [Composition over Inheritance](#composition-over-inheritance)
   5. [Law of Demeter](#law-of-demeter)
   6. [Interface Segregation](#interface-segregation)
   7. [Open/Closed Principle (OCP)](#openclosed-principle-ocp)
   8. [DRY and Modularity](#dry-and-modularity)
5. [Where these principles appear in the code](#where-these-principles-appear-in-the-code)
6. [Why this design works for the project](#why-this-design-works-for-the-project)

---

## OOP Overview

This project uses C++ modules, namespaces, and typed structs to express object-oriented design. It does not rely heavily on class inheritance, but it does apply the core OOP ideas:

- abstraction: expose only the operations that other code needs,
- encapsulation: hide implementation details behind API boundaries,
- data modeling: represent business entities as objects,
- modular design: keep related behavior and state together.

The result is a layered system where each component has a clear role and the code is easier to understand, test, and extend.

---

## How the project expresses OOP principles

### Abstraction

Abstraction is visible in the way modules expose narrow APIs and hide database and implementation details. Examples:

- `DB` abstracts SQLite operations behind a simple query/execute interface.
- `UserModel`, `AccountModel`, and `TransactionModel` abstract raw SQL queries behind typed methods like `findByUsername`, `create`, and `updateStatus`.
- `Auth`, `Transfer`, `Approval`, and `UserMenu` hide the details of how login, payment requests, approval, and menus work.

This means the rest of the code does not need to know how the database is implemented, how SQL is written, or how hashing happens.

### Encapsulation

Encapsulation is enforced by keeping implementation details inside modules and exposing only the necessary interfaces.

Examples:

- `database/db.h` exposes `DB` operations, while the SQLite-specific code stays in `src/database/db.cpp`.
- `models/*.h` expose only the data object definitions and model functions; callers do not manipulate SQL directly.
- `auth/password.h` exposes only `Password::hash` and `Password::verify`, not the hashing algorithm.
- `auth/session.h` exposes only `Session::start`, `Session::end`, `Session::isLoggedIn`, and `Session::current`.

Each module owns its data and behavior, reducing accidental dependencies and making each component easier to reason about.

### Inheritance

The current project uses little to no classical inheritance. Instead, it favors namespace-based modularity and composition.

This is a valid OOP design choice when the domain does not require polymorphic subtypes. The project demonstrates OOP by modeling objects and behaviors, even though it does not define inheritance hierarchies.

### Polymorphism

There is no heavy use of runtime polymorphism via virtual functions in this repository.

However, the project still benefits from polymorphism in a broader sense:

- module APIs behave consistently across different implementations.
- the same conceptual operation, such as "find a record" or "show a menu," is available through multiple modules with consistent naming and semantics.

This is a kind of interface polymorphism: different modules share a common pattern of operation without inheriting from a common base.

---

## Core project objects and modules

### Data objects

The project models its data as plain typed objects:

- `User` represents a system user and includes identity, credentials, role, and creation time.
- `Account` represents a bank account, including account number, balance, and owner.
- `Transaction` represents a transfer request, including source/destination accounts, amount, status, and review metadata.
- `SessionData` represents the authenticated runtime user context.

These objects are used to carry data between layers without exposing raw database rows.

### Service and behavior modules

The project organizes behavior into modules represented by namespaces:

- `DB` is the service object for persistence.
- `Schema` initializes the database.
- `UserModel`, `AccountModel`, and `TransactionModel` provide model services.
- `Auth` provides authentication services.
- `Password` provides credential hashing and verification.
- `Session` provides runtime session management.
- `UserMenu`, `Transfer`, and `UserAccount` provide user-facing behavior.
- `AdminMenu`, `Approval`, and `UserMgmt` provide admin-facing behavior.

Each module acts like an object with methods and responsibilities.

---

## Design principles followed

### Single Responsibility Principle (SRP)

The project follows SRP by giving each module one clear responsibility.

Examples:

- `DB` only manages database access.
- `Schema` only creates tables and seeds data.
- `UserModel` only operates on users.
- `AccountModel` only operates on accounts.
- `TransactionModel` only operates on transactions.
- `Auth` only handles login and registration.
- `Session` only manages session state.
- `Transfer` only handles sending money requests.
- `Approval` only handles transaction review.
- `UserMgmt` only handles user reporting.

When a module does one thing, it is easier to change and reason about.

### Separation of Concerns

The code separates concerns into layers:

- presentation and menu flow in `src/main.cpp`, `user/*`, and `admin/*`
- authentication logic in `auth/*`
- business data and persistence in `models/*`
- low-level database access in `database/*`

This separation ensures that UI changes do not require database changes, and database changes do not require menu changes.

### Dependency Direction / Layering

The project uses a clean dependency direction:

- presentation depends on authentication and user/admin modules,
- authentication depends on models and session/password modules,
- models depend on the database wrapper,
- the database wrapper depends only on SQLite and C++ standard utilities.

Each layer depends only on the layer below it, which keeps coupling low and makes the system more maintainable.

### Composition over Inheritance

Even though the code is in C++, it avoids unnecessary inheritance and instead composes functionality through module calls.

For example:

- `Auth` composes `UserModel`, `AccountModel`, `Password`, and `Session` instead of inheriting from them.
- `Approval` composes `TransactionModel`, `AccountModel`, and `Session`.
- `UserMenu` composes `Transfer` and `UserAccount`.

This design is cleaner for this application because there are no shared base classes or polymorphic hierarchies required by the domain.

### Law of Demeter

The project largely follows the Law of Demeter ("only talk to your immediate friends").

Examples:

- `user/transfer.cpp` talks to `Session`, `AccountModel`, and `TransactionModel`, but does not call methods on objects returned by those models in complex chains.
- `Auth` communicates with models and utilities through simple method calls.

This avoids deep dependency chains and reduces coupling between modules.

### Interface Segregation

Each module exposes a small, focused interface.

Examples:

- `auth/session.h` exposes only four functions.
- `include/models/*.h` expose only the operations needed by the rest of the application.
- `include/user/*.h` and `include/admin/*.h` expose only the menu and workflow entry points.

Clients depend only on the operations they use.

### Open/Closed Principle (OCP)

The project is designed so new behavior can be added with minimal changes to existing code.

Examples:

- adding a new account feature means extending `user/` with another module and menu option rather than changing `Auth` or `DB`.
- adding an admin report means extending `admin/` without changing `Transfer` or `UserAccount`.
- adding a new model entity means adding a new model namespace and new schema definitions, while leaving the existing layer structure intact.

This keeps the system open to extension but closed to invasive modification.

### DRY and Modularity

The design avoids repeated SQL and repeated menu logic.

Examples:

- query logic is centralized in model methods.
- menu loops are separated by role.
- session management is centralized in one module.

Modularity makes the code easier to maintain and reduces the risk of inconsistencies.

---

## Where these principles appear in the code

### `include/database/db.h` and `src/database/db.cpp`

- abstraction: `DB` hides SQLite details.
- encapsulation: SQLite connection state and query implementation are private to the module.
- SRP: this module exists only for persistence.

### `include/models/*.h` and `src/models/*.cpp`

- data modeling: `User`, `Account`, and `Transaction` are typed business objects.
- abstraction: models provide read/write methods instead of raw SQL.
- interface segregation: each model has a focused API.

### `include/auth/*` and `src/auth/*`

- authentication is isolated from both UI and data layers.
- `Password` encapsulates hashing.
- `Session` encapsulates runtime user state.
- `Auth` composes lower-level services without exposing implementation details.

### `include/user/*` and `src/user/*`

- user workflows are separated into `Transfer` and `UserAccount`.
- `UserMenu` coordinates user actions but does not implement account or transaction logic directly.
- this keeps presentation separate from business rules.

### `include/admin/*` and `src/admin/*`

- admin workflows are separated into approval and user reporting.
- `Approval` is responsible for transactional integrity and business rules.
- `UserMgmt` is responsible for reporting only.

---

## Why this design works for the project

This project’s design is a pragmatic application of OOP principles. It uses C++ types, structs, namespaces, and layer-based modules to enforce clean boundaries while keeping the code simple and easy to extend.

The main benefits are:

- easier maintenance through modular APIs,
- clearer behavior through responsibility separation,
- safer database access through model encapsulation,
- better extension ability for new user/admin features,
- reduced coupling between UI, auth, and persistence,
- more predictable change impact because each module owns its own concerns.

Example extension scenarios:

- adding a new account type would likely add `models/account_type.h` and new menu options in `user/` without changing `Auth` or `DB`.
- adding a new authentication policy could extend `auth/auth.cpp` and `auth/password.cpp`, leaving `models/` untouched.
- adding a new admin report would add a new `admin/` module and menu item while leaving user workflows unchanged.

Even without classical inheritance or heavy runtime polymorphism, this project demonstrates object-oriented design through well-defined objects, encapsulated behavior, clearly defined module interfaces, and consistent layer boundaries.
