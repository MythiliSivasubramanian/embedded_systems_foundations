# What Happens When We Switch On a Car?

Let's imagine pressing the Start button in a car.

Does the ECU immediately start executing its application logic? No. Before the application can do its actual job, the ECU and its microcontroller have to go through several steps.

Let's understand what happens and why.

## 1. Power supply becomes available

The vehicle battery supplies electrical power to the ECU.

But the battery voltage is not necessarily at the exact level required by every electronic component inside the ECU.

So, the ECU has power supply circuitry that:
- Regulates the incoming voltage to the required levels.
- Monitors conditions such as undervoltage and overvoltage.
- Helps protect the electronic components from abnormal power conditions.

**What I understood:** The ECU needs a suitable and stable power supply before its electronics can operate correctly.

## 2. ECU wakes up

An ECU may be in a low-power or sleep state when the vehicle is switched off.

When the vehicle needs that ECU, it can be woken up through different mechanisms:

- An ignition or wake-up signal.
- Network activity, such as a CAN message.
- An internal timer or another configured wake-up source.

The wake-up mechanism depends on the ECU design.

**Something important I learned:**

Wake-up and reset are not necessarily the same thing.

An ECU can wake up from a low-power state without performing a complete microcontroller reset. The behaviour depends on how the ECU and its power-management circuitry are designed.

## 3. Microcontroller reset and startup

Now let's look at the MCU inside the ECU.

Once the microcontroller exits its reset condition, it starts executing its startup sequence.

For an ARM Cortex-M microcontroller, such as the STM32F407, the sequence typically involves:

1. The CPU obtains the initial stack pointer and reset-handler address from the vector table.
2. The reset handler starts executing.
3. RAM sections such as `.data` and `.bss` are initialized.
4. The C runtime environment is prepared.
5. Execution is transferred to the `main()` function.

**Why is this necessary?**

The application needs a properly prepared execution environment. For example, global variables must have their expected initial values before the application uses them.

## 4. Hardware initialization

Reaching `main()` does not mean all the hardware is automatically ready.

The firmware must configure the peripherals required by the application.

For example:

- Configure system and peripheral clocks.
- Enable the clocks for the required peripherals.
- Configure GPIO pins.
- Initialize communication interfaces such as CAN, UART, SPI and I2C.
- Configure timers and interrupts.
- Initialize other hardware components required by the application.

### Something I learned about GPIO

Let's say we want to use GPIOA on the STM32F407.

There are two separate operations:

**First: Enable the GPIOA peripheral clock.**

This allows the GPIOA peripheral to operate.

**Second: Configure the individual GPIOA pin.**

We must decide whether a pin should work as:
- Input
- Output
- Alternate function
- Analog

Enabling the GPIOA clock does not automatically make all its pins outputs.

Also, the GPIO clock is not the electrical signal sent through the pin. The GPIO output circuitry generates the actual HIGH or LOW signal.

## 5. Application execution

Once the required startup and initialization steps are complete, the application can begin its actual work.
Depending on the ECU architecture, the application may use:

**Superloop architecture**

The application repeatedly executes operations inside a main loop.

**RTOS architecture**

An RTOS (Real-Time Operating System) manages multiple tasks and schedules their execution.

One important distinction:

- Startup code prepares the software execution environment.
- Hardware initialization configures the peripherals.
- Application execution performs the intended control functionality.

Also, reaching `main()` does not necessarily mean normal runtime operation starts immediately. The application may perform additional initialization before entering its normal operating loop or starting the RTOS scheduler.

## 6. Input processing

Now the ECU is ready to perform its actual job.

It receives information from different sources:

- Sensors connected directly to the ECU.
- Messages received from other ECUs through communication networks such as CAN.

For example, an engine-control ECU may receive engine-speed and temperature measurements.

The MCU processes this information using the firmware running on it.

## 7. Output control

Based on the received information and application logic, the ECU performs the required actions.

These may include:

- Controlling actuators.
- Sending messages to other ECUs.
- Monitoring system conditions.
- Detecting and handling faults.

For example, an engine-control ECU may determine the required fuel-injection and ignition timing commands based on sensor information.

Not every ECU controls the engine. A Body Control Module (BCM), for example, may manage vehicle lighting, power windows and door locks.

## Initialization vs. Runtime Operation

I find it useful to divide the ECU's software operation into two broad stages.

**Initialization**

Preparing the microcontroller, hardware peripherals and software environment before normal application operation.

**Runtime operation**

Repeatedly reading inputs, processing information, controlling outputs and monitoring system behaviour.

### My understanding of the complete sequence

Power supply -> ECU wake-up -> MCU reset/startup (when applicable) -> Hardware initialization -> Application execution -> Input processing -> Output control.

### Takeaways:

An ECU is a combination of electronic hardware and embedded software.

The MCU is responsible for executing the firmware, but the complete ECU needs supporting hardware to supply power, communicate with other systems, receive sensor signals and control actuators.

Understanding this sequence helps me see how hardware and software work together inside an automotive ECU.