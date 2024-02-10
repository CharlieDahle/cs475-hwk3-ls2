#include <stdlib.h>
#include <stdio.h>
#include "stack.h"
#include "ls2.h"

/**
 * Main function
 * Usage: ls2 <path> [exact-match-pattern]
 */
int main(int argc, char *argv[])
{

	// stack_t *s = initstack();

	// Check for the correct number of arguments
	if (argc < 2 || argc > 3)
	{
		printf("Usage: %s <path> [exact-match-pattern]\n", argv[0]);
		return 1; // Exit with a non-zero status to indicate an error
	}

	// If the optional [exact-match-pattern] is provided, it will be in argv[2]
	const char *matchPattern = NULL;
	if (argc == 3)
	{
		matchPattern = argv[2];
	}

	// Call the viewDir function with the provided path and optional match pattern
	// Starting at depth 0 for the top-level directory
	configure(argv[1], matchPattern);

	return 0; // Exit with 0 to indicate success
}
