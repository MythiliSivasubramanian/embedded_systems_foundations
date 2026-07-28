# Understanding `typedef`, Pointers, Macros and the `->` Operator in Embedded C

In the previous chapter, we learnt how to model an entire GPIO peripheral using a C structure. The structure described the hardware register layout, and the pointer connected that structure to the actual peripheral base address.

However, there are still a few C concepts that are used together in almost every STM32 project:

* `typedef`
* Structure Objects
* Structure Pointers
* Type Casting
* Macros
* The `->` operator

These concepts often appear together, making them confusing at first. In this chapter, we will understand each concept individually and then see how they all come together when accessing STM32 peripherals.

---

# 1. Why do we use `typedef`?

Consider the following structure definition:

```c
struct GPIO
{
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
};
```

Here, the compiler creates a new structure type called:

```c
struct GPIO
```

To create an object of this structure, we must write:

```c
struct GPIO gpio1;
```

Notice that every time we create an object, we must write the keyword `struct`.

To make this easier, C provides the `typedef` keyword.

Example:

```c
typedef struct
{
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
} GPIO_TypeDef;
```

Now the compiler creates a new type called:

```c
GPIO_TypeDef
```

Instead of writing:

```c
struct GPIO gpio1;
```

we simply write:

```c
GPIO_TypeDef gpio1;
```

The keyword `struct` is no longer required.

---

# 2. What exactly does `typedef` do?

The important point is:

`typedef` only creates a new name (alias) for an existing type.

It **does not**:

* allocate memory
* create an object
* create a pointer

It only creates another type name.

For example:

```c
typedef struct
{
    int age;
    int marks;
} Student;
```

Here,

```c
Student
```

is only a type.

At this point:

* No object exists.
* No memory has been allocated.

---
