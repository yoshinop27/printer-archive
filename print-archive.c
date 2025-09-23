#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_contents(uint8_t *data, size_t size);

int main(int argc, char **argv) {
  // Make sure we have a file input
  if (argc != 2) {
    fprintf(stderr, "Please specify an input filename.\n");
    exit(1);
  }

  // Try to open the file
  FILE *input = fopen(argv[1], "r");
  if (input == NULL) {
    perror("Unable to open input file");
    exit(1);
  }

  // Seek to the end of the file so we can get its size
  if (fseek(input, 0, SEEK_END) != 0) {
    perror("Unable to seek to end of file");
    exit(1);
  }

  // Get the size of the file
  size_t size = ftell(input);

  // Seek back to the beginning of the file
  if (fseek(input, 0, SEEK_SET) != 0) {
    perror("Unable to seek to beginning of file");
    exit(2);
  }

  // Allocate a buffer to hold the file contents. We know the size in bytes, so
  // there's no need to multiply to get the size we pass to malloc in this case.
  uint8_t *data = malloc(size);

  // Read the file contents
  if (fread(data, 1, size, input) != size) {
    fprintf(stderr, "Failed to read entire file\n");
    exit(2);
  }

  // Make sure the file starts with the .ar file signature
  if (memcmp(data, "!<arch>\n", 8) != 0) {
    fprintf(stderr, "Input file is not in valid .ar format\n");
    exit(1);
  }

  // Call the code to print the archive contents
  print_contents(data, size);

  // Clean up
  free(data);
  fclose(input);

  return 0;
}

// trim whitespace from header values
void trim_whitespace(char *str) {
  char *end = str + strlen(str) - 1;
  while (end > str && ((*end == ' ') || (*end == '/'))) {
    end--;
  }
  *(end + 1) = '\0';
}

/**
 * This function should print the name of each file in the archive followed by
 * its contents.
 *
 * \param data This is a pointer to the first byte in the file.
 * \param size This is the number of bytes in the file.
 */
void print_contents(uint8_t *data, size_t size) {
  // Declare CONSTANTS for sizes
  const size_t SIG = 8;
  // Start Cursor after signature
  size_t cursor = SIG;
  // Loop through the archive
  while (cursor + 60 <= size) {
    // Place to hold name
    char* name = malloc(17);
    // Read the name
    for (int j = 0; j < 16; j++) {
      *(name+j) = data[cursor + j];
    }
    *(name+16) = '\0';
    // Trim whitespace from name
    trim_whitespace(name);

    // Place to hold size
    char* size_str = malloc(11);
    // Read the size
    for (int k = 0; k < 11; k++) {
      *(size_str+k) = data[cursor + 48 + k];
    }
    *(size_str + 10) = '\0';
    // trim and turn to int
    trim_whitespace(size_str);
    size_t size_value = atoi(size_str);

    // Move the header to the value
    cursor += 60;
    // Check to make sure data doesnt go over size
    if (size < cursor + size_value) return;
    // Create an string to hold chars
    char *value = malloc(size_value+1);

    // Read the values
    for (size_t j = 0; j < size_value; j++) {
      *(value + j) = data[cursor + j];
    }
    *(value+size_value) = '\0';

    // Move cursor, check if odd
    if (size_value % 2 != 0) {
      size_value++;
    }
    cursor += size_value;
    // Print the name and contents
    printf("%s\n", name);
    printf("%s\n", value);

    //free memory
    free(name);
    free(size_str);
    free(value);
  }
}
