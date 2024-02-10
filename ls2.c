#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ls2.h"

/**
 * Recursively lists all files and directories starting from the specified directory,
 * emulating the `ls` command with the added functionality of traversing into subdirectories.
 * Each file and directory is printed with indentation corresponding to its depth in the directory tree,
 * providing a visual hierarchy of the file system structure.
 *
 * @param dirInput The path of the starting directory from which the listing begins.
 * @param depth The current level of depth in the directory tree, used for indentation. The top-level directory starts at depth 0.
 */
void mode1(char *dirInput, int depth)
{
    DIR *dir;             // Pointer to the directory stream
    struct dirent *entry; // Structure representing directory entry
    struct stat fileStat; // Structure holding information about a file system entry

    // Attempt to open the specified directory
    dir = opendir(dirInput);
    if (dir == NULL) // Check if the directory could not be opened
    {
        perror("opendir"); // Print an error message if failed to open directory
        return;            // Exit the function if the directory cannot be opened
    }

    // Loop through each entry in the directory
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip '.' and '..' entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue; // Move to the next directory entry
        }

        // Construct the full path of the current entry
        char *fullPath = malloc(strlen(dirInput) + strlen(entry->d_name) + 2); // Allocate memory for the full path
        if (fullPath == NULL)                                                  // Check if memory allocation failed
        {
            perror("malloc"); // Print an error message if memory allocation fails
            continue;         // Skip this entry and continue with the next one
        }

        sprintf(fullPath, "%s/%s", dirInput, entry->d_name); // Format the full path string

        // Retrieve the status of the current file system entry
        if (lstat(fullPath, &fileStat) == 0) // Check if the status was successfully retrieved
        {
            // Print indentation corresponding to the depth in the directory tree
            for (int i = 0; i < depth; i++)
            {
                printf("    "); // 4 spaces for each level of depth
            }

            // Print the entry's name and additional info based on its type
            if (S_ISREG(fileStat.st_mode)) // If the entry is a regular file
            {
                printf("%s (%ld bytes)\n", entry->d_name, fileStat.st_size); // Print file name and size
            }
            else if (S_ISDIR(fileStat.st_mode)) // If the entry is a directory
            {
                printf("%s/ (directory)\n", entry->d_name); // Print directory name
                mode1(fullPath, depth + 1);                 // Recursively list contents of the directory, increasing depth
            }
        }
        else
        {
            perror("lstat"); // Print an error message if failed to retrieve entry status
        }

        free(fullPath); // Free the memory allocated for the full path
    }
    closedir(dir); // Close the directory stream
}

/**
 * Recursively searches directories for files that match a given keyword and stores their paths in a stack.
 * It formats paths with indentation based on their depth in the directory tree to preserve the hierarchy.
 *
 * @param dirInput The starting directory for the search.
 * @param depth The depth of the current directory from the starting point, used for indentation.
 * @param keyword The filename to search for.
 * @param s The stack where matching file paths are stored.
 */
int mode2(char *dirInput, int depth, char *keyword, stack_t *s)
{
    DIR *dir;             // Directory stream.
    struct dirent *entry; // Directory entry structure.
    struct stat fileStat; // File status structure.
    int hasMatchingDescendant = 0;

    // Attempt to open the directory.
    dir = opendir(dirInput);
    if (!dir)
    {
        perror("opendir"); // Print an error if the directory can't be opened.
        return 0;
    }

    // Iterate through each entry in the directory.
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip "." and ".." entries to prevent loops.
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Allocate memory for the full path of the current entry and check for allocation errors.
        char *fullPath = malloc(strlen(dirInput) + strlen(entry->d_name) + 2); // +2 for the slash and null terminator.

        if (!fullPath)
        {
            perror("malloc");
            continue;
        }

        // Construct the full path of the current entry.
        sprintf(fullPath, "%s/%s", dirInput, entry->d_name);
        // printf("\n\nFull Path: %s\n\n", fullPath);

        // Retrieve information about the current entry. Skip it if an error occurs.
        if (lstat(fullPath, &fileStat) == -1)
        {
            perror("lstat");
            free(fullPath); // Free the allocated memory for the path.
            continue;
        }

        // Check if the current entry is a directory.
        if (S_ISDIR(fileStat.st_mode))
        {
            // Recursively search within the subdirectory
            if (mode2(fullPath, depth + 1, keyword, s))
            {
                hasMatchingDescendant = 1; // Use 1 as true
            }
            free(fullPath); // Free the allocated memory for the directory path after recursion
        }
        else if (S_ISREG(fileStat.st_mode) && strcmp(entry->d_name, keyword) == 0)
        {
            hasMatchingDescendant = 1; // Found a matching file and use 1 as true

            // Format and push the matching file path onto the stack
            char *formattedFilePath = malloc(strlen(fullPath) + depth * 4 + 1);
            if (formattedFilePath)
            {
                snprintf(formattedFilePath, strlen(fullPath) + depth * 4 + 1, "%*s%s", depth * 4, "", fullPath);
                push(s, formattedFilePath);
            }
            free(fullPath); // Free the allocated memory for the file path
        }
        else
        {
            free(fullPath); // Free the allocated memory if it's neither a matching file nor a directory
        }
    }

    closedir(dir);

    // Only add the directory to the stack if it has a matching descendant
    if (hasMatchingDescendant)
    {
        char *formattedDirPath = malloc(strlen(dirInput) + depth * 4 + 2); // Allocate space for the formatted directory path
        if (formattedDirPath)
        {
            snprintf(formattedDirPath, strlen(dirInput) + depth * 4 + 2, "%*s%s/", depth * 4, "", dirInput);
            push(s, formattedDirPath); // Push the formatted directory path onto the stack
        }
    }

    return hasMatchingDescendant; // Return whether this directory or any subdirectory had a matching file
}

/**
 * Configures and initiates the search for files matching a specific pattern or lists all files and directories.
 * If a match pattern is provided, it searches for files matching the pattern and prints their paths.
 * Otherwise, it lists all files and directories from the given starting directory.
 *
 * @param dirInput The starting directory for listing or searching.
 * @param matchPattern The pattern to match filenames against, or NULL to list all files and directories.
 */
void configure(char *dirInput, char *matchPattern)
{
    stack_t *s = initstack(); // Initialize the main stack for storing paths.

    if (matchPattern == NULL)
    {
        // If no match pattern is provided, list all files and directories using mode1.
        mode1(dirInput, 0);
    }
    else
    {
        // If a match pattern is provided, search for matching files using mode2.
        printf("Looking for: %s\n", matchPattern);
        mode2(dirInput, 0, matchPattern, s);

        printstack(s);
        freestack(s);
    }
}
