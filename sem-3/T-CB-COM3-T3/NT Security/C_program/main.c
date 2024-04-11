#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        printf("Invalid number of arguments given. Need at least 3 (positional) arguments.\n");
        printf("xor_crypto <input file> <password> <output file>.\n");
        return EXIT_FAILURE;
    }
    FILE *test = fopen(argv[3], "r");
    if (test != NULL)
    {
        fclose(test);
        remove(argv[3]);
    }
    const char* password = argv[2];
    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[3], "w+");
    if (input == NULL)
    {
        printf("Could not open input file. Aborting.\n");
        return EXIT_FAILURE;
    }
    else if (output == NULL)
    {
        printf("Could not create output file. Aborting.\n");
        return EXIT_FAILURE;
    }
    size_t read = 0;
    const size_t bufferSize = 4 *1024;
    char buffer[bufferSize];
    size_t passwordIndex = 0;
    while((read = fread(buffer, sizeof(char), bufferSize, input)) > 0)
    {
        for (size_t i = 0; i < read; i++)
        {
            buffer[i] = buffer[i] ^ password[passwordIndex++];
            passwordIndex = passwordIndex % passwordIndex;
        }
        fwrite(buffer, sizeof(char), read, output);
    }
    fclose(input);
    fclose(output);
    return EXIT_FAILURE;
}