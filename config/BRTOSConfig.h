///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/////                                                     /////
/////                   OS User Defines                   /////
/////                                                     /////
/////             !User configuration defines!            /////
/////                                                     /////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

// Include simulator
#include "cooja.h"

/// Define if simulation or DEBUG
#define DEBUG 1

/**
 * Configure processor endiannes (see MCU datasheet)
 * BRTOS_BIG_ENDIAN              (0)
 * BRTOS_LITTLE_ENDIAN           (1)
 */
#define ENDIANNESS	1

/// Define if verbose info is available
#define VERBOSE 0			/// TODO: code crash if it's enabled, why?
							/// INFO: some tasks initialize,
							///       but a some short time the simulation stop,
							///       the PC (Program Counter) got lost

/// Define if error check is available
#define ERROR_CHECK 0

/// Define if whatchdog is active
#define WATCHDOG 1

/// Define if compute cpu load is active
#define COMPUTES_CPU_LOAD 1		// TODO: uNet, check if it's necessary  (uNet = 1)

#define ostick_t				uint64_t
#define osdtick_t				uint64_t

// The Nesting define must be set in the file HAL.h
// Example:
/// Define if nesting interrupt is active
//#define NESTING_INT 0

#define TASK_WITH_PARAMETERS 0 	// TODO: uNet, check if it's necessary  (uNet = 1)
								// INFO: port doesn't work if it's 1, don't know why;
								//		 probably the stack is changed with the parameters of the function
								//		 must change the OS_SAVE_ISR function to better one

/// Define Number of Priorities
#define NUMBER_OF_PRIORITIES 31

/// Define CPU Stack Pointer Size
#define SP_SIZE 32

/* stacked by the RTI interrupt process */
// R4-R15 (16 up to 20 bits) (24 up to 48 bytes) + SR (12 bits) + SP (20 bits)
#define NUMBER_MIN_OF_STACKED_BYTES	52  // (R4-R15) [48 B] + (SR + PC) [4 B]
/// Minimum size of a task stack.

// User defined: stacked for user function calls + local variables
// Ainda, como podem ocorrer interrucoes durante as tarefas, alocar 28 bytes a cada
// interrupcao ativa


/// Define the maximum number of Tasks to be Installed
/// must always be equal or higher to NumberOfInstalledTasks
#define NUMBER_OF_TASKS (INT8U)9 // TODO check this number for uNET  (uNet = 12)

/// Define if OS Trace is active
#define OSTRACE 0

#if (OSTRACE == 1)  
  #include "debug_stack.h"
#endif

/// Enable or disable the dynamic task install and uninstall
#define BRTOS_DYNAMIC_TASKS_ENABLED 0

/// Define if TimerHook function is active
#define TIMER_HOOK_EN 			0

/// Define if IdleHook function is active
#define IDLE_HOOK_EN 			0

/// Enable or disable timers service
#define BRTOS_TMR_EN			1

/// Enable or disable semaphore controls
#define BRTOS_SEM_EN			1

/// Enable or disable binary semaphore controls
#define BRTOS_BINARY_SEM_EN		1

/// Enable or disable mutex controls
#define BRTOS_MUTEX_EN			1

/// Enable or disable mailbox controls
#define BRTOS_MBOX_EN			0

/// Enable or disable queue controls
#define BRTOS_QUEUE_EN			1

/// Enable or disable dynamic queue controls
#define BRTOS_DYNAMIC_QUEUE_ENABLED	0 // TODO: uNet, uses it? (uNet = 1)

/// Enable or disable queue 16 bits controls
#define BRTOS_QUEUE_16_EN      0

/// Enable or disable queue 32 bits controls
#define BRTOS_QUEUE_32_EN      0

/// Defines the maximum number of semaphores\n
/// Limits the memory allocation for semaphores
#define BRTOS_MAX_SEM          13 // 13 counted

/// Defines the maximum number of mutexes\n
/// Limits the memory allocation for mutex
#define BRTOS_MAX_MUTEX        4

/// Defines the maximum number of mailboxes\n
/// Limits the memory allocation mailboxes
#define BRTOS_MAX_MBOX         5

/// Defines the maximum number of queues\n
/// Limits the memory allocation for queuesAPP3_Priority
#define BRTOS_MAX_QUEUE        1


/// TickTimer Defines
#define configCPU_CLOCK_HZ          16000000uL    ///< CPU clock in Hertz
#define configCPU_ACLK_HZ			32768uL       ///< ACLK clock in Hertz
#define configTICK_RATE_HZ          1000        ///< Tick timer rate in Hertz
#define configTIMER_PRE_SCALER      0                   ///< Informs if there is a timer prescaler
#define configRTC_CRISTAL_HZ        1000
#define configRTC_PRE_SCALER        10
#define OSRTCEN                     0
//#define INTERVAL					(configCPU_CLOCK_HZ / configTICK_RATE_HZ) >> configTIMER_PRE_SCALER
#define INTERVAL 8000

// Stack Size of the Idle Task
#define IDLE_STACK_SIZE     4*NUMBER_MIN_OF_STACKED_BYTES	/// double of minimum stack size

/// Stack Defines
/// msp430f2274 1KB RAM (total)
/// msp430f2617 8kb RAM /// msp430f5437 16kb RAM
#define HEAP_SIZE 			2*4096

// Queue heap defines
// Configurado com 32 bytes p/ filas (para filas)
#define QUEUE_HEAP_SIZE 	512

// Dynamic head define. To be used by DynamicInstallTask and Dynamic Queues
//#define DYNAMIC_HEAP_SIZE	1*1024

//** Enable/Disable printf standard library **//
//** Must be configured as project specifications**//
#define PRINTF_EN 		1
#define PRINTF_ARGS_EN 	1

#if PRINTF_EN
#if PRINTF_ARGS_EN
#include "stdio.h"
#include "assert.h"
#else
#include "drivers.h"
#define PRINTF(__STRING__, ...) puts(__STRING__);
#define assert(x) if(!(x)){ PRINTF("Error: assert trap!\n"); while(1); }
#endif // STDIO_PRINTF_EN
#else
#define PRINTF(...)
#endif // PRINTF_EN

