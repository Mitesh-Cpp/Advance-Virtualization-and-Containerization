#include <stddef.h>
#include <stdint.h>

// declaring global character arrays for storing the result of hypercall HC_numExitsByType
char s0[100], s1[100], s2[100], s3[100], s4[100], s5[100], s6[100], s7[100], s8[100], s9[100];
// turn variable used for handling multiple calls for HC_numExitsByType. Each time it is called, it should use new character array.
int turn = 0;

static inline void outb(uint16_t port, uint8_t value) {
	asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

// using "inl" and "outl" as suggested in the assignment document

static inline void outl(uint16_t port, uint32_t value) {
	asm("outl %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

static inline uint32_t inl(uint16_t port) {
	uint32_t value;
	// Take the input in the variable "value" from the hyperviser and return its
    asm("in %1, %0" : /* empty */ "=a"(value) : "Nd"(port) : "memory" );
	return value;
}

void HC_print8bit(uint8_t val)
{
	outb(0xE9, val);
}

void HC_print32bit(uint32_t val) 
{
	// Fill in here
	outl(0xF1, val);
}

uint32_t HC_numExits()
{
	// Fill in here
	return inl(0xF4);
}

void HC_printStr(char *str) {
	// Fill in here
	// Sending the address of string to the hypervisor and making hyperviser print the string on terminal
    uint32_t address = (uint32_t)(uintptr_t)str;
    outl(0xF2, address);
}

char *HC_numExitsByType()
{   
	// Fill in here
	// In case, someone tries to run it multiple times ;-)
	// used simple logic of turn here, that uses all the available character arrays in the cyclic manner due to modulo
	// type casting done for sending the address and receiving it as outl expects uint_32 type and function expects the return type of char*
	uint32_t address;
	switch(turn) {
		case 0: address = (uint32_t)(uintptr_t)s0;
				break;
		case 1: address = (uint32_t)(uintptr_t)s1;
				break;
		case 2: address = (uint32_t)(uintptr_t)s2;
        		break;
		case 3: address = (uint32_t)(uintptr_t)s3;
				break;
		case 4: address = (uint32_t)(uintptr_t)s4;
				break;
		case 5: address = (uint32_t)(uintptr_t)s5;
				break;
		case 6: address = (uint32_t)(uintptr_t)s6;
				break;
		case 7: address = (uint32_t)(uintptr_t)s7;
				break;
		case 8: address = (uint32_t)(uintptr_t)s8;
				break;
		case 9: address = (uint32_t)(uintptr_t)s9;
				break;
		default:
				address = (uint32_t)(uintptr_t)s0;
	}
	turn = (turn+1)%10;
	outl(0xFF, address);
	// a bit of a redundant logic, that inputs the same address which was sent using outl. Using inl is redundant here. 
	// Noticed this in the end, and decided not to change it last minute. ;-)
	uint32_t ret_add = inl(0xF3);
	void* ret_add_ptr = (void*)(uintptr_t)ret_add;
    char* char_ptr = (char*) ret_add_ptr;
	return char_ptr;
}

uint32_t HC_gvaToHva(uint32_t gva)
{
	// Fill in here
	// using outl to send the gva to hyperviser and inl to receive the converted hva from hyperviser
	outl(0xEE, gva);
	return inl(0xEF);
}

void
__attribute__((noreturn))
__attribute__((section(".start")))
_start(void) {
	const char *p;

	for (p = "Hello 695!\n"; *p; ++p)
		HC_print8bit(*p);


	/*----------Don't modify this section. We will use grading script---------*/
	/*---Your submission will fail the testcases if you modify this section---*/
	HC_print32bit(2048);
	HC_print32bit(4294967295);

	uint32_t num_exits_a, num_exits_b;
	num_exits_a = HC_numExits();

	char *str = "CS695 Assignment 2\n";
	HC_printStr(str);

	num_exits_b = HC_numExits();

	HC_print32bit(num_exits_a);
	HC_print32bit(num_exits_b);

	char *firststr = HC_numExitsByType();
	uint32_t hva;
	hva = HC_gvaToHva(1024);
	HC_print32bit(hva);
	hva = HC_gvaToHva(4294967295);
	HC_print32bit(hva);
	char *secondstr = HC_numExitsByType();

	HC_printStr(firststr);
	HC_printStr(secondstr);
	/*------------------------------------------------------------------------*/

	*(long *) 0x400 = 42;

	for (;;)
		asm("hlt" : /* empty */ : "a" (42) : "memory");
}
