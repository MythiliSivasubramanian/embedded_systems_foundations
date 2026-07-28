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

# 3. Type vs Object

This is one of the most important concepts in C.

After writing:

```c
typedef struct
{
    int age;
    int marks;
} Student;
```

The compiler knows that a type called `Student` exists.

Now we create an object:

```c
Student s1;
```

Now memory is allocated.

Conceptually:

```text
Memory

s1

+---------+
| age     |
+---------+
| marks   |
+---------+
```

Therefore,

* `Student` → Type
* `s1` → Object (variable)

The object occupies memory.
The type does not.

---

# 4. Structure Pointer

Instead of creating another structure object, suppose we create a pointer.

```c
Student *ptr;
```

This does **not** create another Student object.

It only creates a pointer capable of storing the address of a Student object.

Initially:

```text
ptr

+------------+
|    ????    |
+------------+
```

Now suppose:

```c
ptr = &s1;
```

Memory becomes:

```text
ptr

+------------+
| 0x1000     |
+------------+
        |
        |
        v

s1

+---------+
| age     |
+---------+
| marks   |
+---------+
```

Notice carefully:

The pointer is **not** the Student object.

It only stores the address of the Student object.

---

# 5. Accessing Members Through a Pointer

Suppose we have:

```c
Student s1;
Student *ptr = &s1;
```

If we want to access `age`, can we write:

```c
ptr.age = 20;
```

No.

Why?

Because `ptr` is not a structure object.

It is only a pointer.

First, we must access the object that the pointer points to.

In C, this is done using the dereference operator (`*`).

```c
(*ptr).age = 20;
```

Let's understand this step by step.

```c
*ptr
```

means:

Go to the Student object whose address is stored in `ptr`.

Once we have the structure object, we can use the `.` operator.

```c
(*ptr).age
```

means:

Go to the Student object pointed to by `ptr`, then access its `age` member.

---

# 6. Why does the `->` operator exist?

Writing

```c
(*ptr).age
```

every time is inconvenient.

Therefore, C provides a shortcut.

Instead of writing:

```c
(*ptr).age
```

we simply write:

```c
ptr->age
```

Both statements are exactly the same.

```c
ptr->age
```

is simply shorthand for

```c
(*ptr).age
```

---

# 7. Applying this to STM32 GPIO

Suppose we define our GPIO structure as:

```c
typedef struct
{
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;

} GPIO_TypeDef;
```

This only defines the layout of a GPIO peripheral.

It does **not** create GPIOA.

It does **not** allocate memory for the registers.

It simply tells the compiler how a GPIO peripheral is organised.

---

# 8. Connecting the Structure to Hardware

Suppose GPIOB base address is:

```text
0x40020400
```

We can create a pointer:

```c
GPIO_TypeDef *GPIOB =
        (GPIO_TypeDef *)0x40020400U;
```

Let's understand both sides.

### Left-hand side (LHS)

```c
GPIO_TypeDef *GPIOB;
```

declares a pointer capable of storing the address of a `GPIO_TypeDef`.

### Right-hand side (RHS)

```c
0x40020400
```

is simply a hexadecimal number.

To tell the compiler that this number should be treated as the address of a GPIO peripheral, we type cast it.

```c
(GPIO_TypeDef *)0x40020400U
```

Now the compiler knows:

> Treat address `0x40020400` as a pointer to a `GPIO_TypeDef`.

---

# 9. Why do STM32 Libraries Use Macros?

Instead of creating pointer variables, STM32 header files define peripherals like this:

```c
#define GPIOA ((GPIO_TypeDef *)0x40020000U)

#define GPIOB ((GPIO_TypeDef *)0x40020400U)
```

Notice that these are **macros**, not variables.

When the preprocessor sees:

```c
GPIOB
```

it simply replaces it with:

```c
((GPIO_TypeDef *)0x40020400U)
```

No memory is allocated.

No pointer variable exists in RAM.

The macro is only a text replacement rule.

This is ideal because peripheral base addresses are fixed by the hardware and never change during program execution.

---

# 10. Understanding `GPIOB->ODR`

Suppose we write:

```c
GPIOB->ODR |= (1U << 5);
```

The macro first expands to:

```c
((GPIO_TypeDef *)0x40020400U)->ODR |= (1U << 5);
```

The `->` operator is equivalent to:

```c
(*GPIOB).ODR
```

The compiler already knows that the `ODR` member is located at offset `0x14`.

Therefore it calculates:

```text
GPIOB Base Address   = 0x40020400

ODR Offset           = 0x14

---------------------------------

ODR Register Address = 0x40020414
```

The CPU finally performs:

* Read the 32-bit value from `0x40020414`
* Set Bit 5
* Write the modified value back to `0x40020414`

The structure does not store the registers.

The structure only describes the register layout.

The pointer connects the structure layout to the real hardware peripheral.

---

# 11. Putting Everything Together

```c
typedef struct
{
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;

} GPIO_TypeDef;


#define GPIOB ((GPIO_TypeDef *)0x40020400U)


int main(void)
{
    GPIOB->ODR |= (1U << 5);

    while(1)
    {

    }
}
```

Flow of execution:

```text
GPIOB
        ↓
Macro Replacement
        ↓
((GPIO_TypeDef *)0x40020400U)
        ↓
Pointer to GPIO Peripheral
        ↓
GPIOB->ODR
        ↓
Base Address + ODR Offset
        ↓
0x40020414
        ↓
CPU accesses the actual hardware register
```

---

# Key Takeaways

* `typedef` creates a new name (alias) for a type.
* `typedef` does not create an object or allocate memory.
* A structure type defines the memory layout of a peripheral.
* A structure object allocates memory.
* A pointer stores the address of an object.
* `*pointer` accesses the object pointed to by the pointer.
* `pointer->member` is shorthand for `(*pointer).member`.
* A type cast tells the compiler how to interpret a memory address.
* STM32 uses macros because peripheral base addresses are fixed and do not require RAM.
* The compiler calculates register addresses using:

```text
Register Address = Peripheral Base Address + Register Offset
```

* The structure describes the peripheral layout, while the pointer (or macro) connects that layout to the actual hardware.

