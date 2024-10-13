#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define FILENAME_SIZE 100
#define BUFFER_SIZE 1024
#define MAX_COLUMNS 10

void CreateTable(const char *Table, const char *Schema[], int schema_size) {
    char filename[FILENAME_SIZE];
    snprintf(filename, sizeof(filename), "%s.txt", Table);
    FILE *file = fopen(filename, "w");

    if (!file) {
        perror("Error creating file");
        return;
    }

    fprintf(file, "%s=[", Table);
    for (int i = 0; i < schema_size; i++) {
        fprintf(file, "\"%s\"", Schema[i]);
        if (i < schema_size - 1) fprintf(file, ",");
    }
    fprintf(file, "]\n{\n");
    fclose(file);
    printf("Created Table '%s' with schema.\n", Table);
}

void Insert(const char *Table, const char *Data[], int data_size) {
    char filename[FILENAME_SIZE];
    snprintf(filename, sizeof(filename), "%s.txt", Table);
    FILE *file = fopen(filename, "a");

    if (!file) {
        perror("Error opening file");
        return;
    }

    fprintf(file, "{");
    for (int i = 0; i < data_size; i++) {
        fprintf(file, "\"%s\"", Data[i]);
        if (i < data_size - 1) fprintf(file, ",");
    }
    fprintf(file, "},\n");
    fclose(file);
    printf("Inserted data into Table '%s'.\n", Table);
}

void CloseTable(const char *Table) {
    char filename[FILENAME_SIZE];
    snprintf(filename, sizeof(filename), "%s.txt", Table);
    FILE *file = fopen(filename, "a");

    if (!file) {
        perror("Error opening file");
        return;
    }

    fprintf(file, "}\n");
    fclose(file);
}

void trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

void Get(const char *Table, const char *column_name, const char *search_value) {
    char filename[FILENAME_SIZE];
    snprintf(filename, sizeof(filename), "%s.txt", Table);
    FILE *file = fopen(filename, "r");

    if (!file) {
        perror("Error opening file");
        return;
    }

    char buffer[BUFFER_SIZE];
    fgets(buffer, sizeof(buffer), file);

    int column_index = -1;
    char *schema = strchr(buffer, '[');
    if (schema) {
        char *token = strtok(schema, "[\",]");
        int index = 0;
        while (token) {
            trim_whitespace(token);
            if (strcmp(token, column_name) == 0) {
                column_index = index;
                break;
            }
            index++;
            token = strtok(NULL, "[\",]");
        }
    }

    if (column_index == -1) {
        printf("Column '%s' not found in table '%s'.\n", column_name, Table);
        fclose(file);
        return;
    }

    int found = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strstr(buffer, "}") && strlen(buffer) <= 3) {
            break;
        }

        char *start = strchr(buffer, '{');
        char *end = strrchr(buffer, '}');
        if (start && end) {
            *end = '\0';
            start++;
            char *record[MAX_COLUMNS];
            int i = 0;
            char *token = strtok(start, "\",");
            while (token && i < MAX_COLUMNS) {
                trim_whitespace(token);
                record[i++] = token;
                token = strtok(NULL, "\",");
            }

            if (i > column_index && strcmp(record[column_index], search_value) == 0) {
                printf("Found record: {");
                for (int j = 0; j < i; j++) {
                    printf("\"%s\"", record[j]);
                    if (j < i - 1) printf(", ");
                }
                printf("}\n");
                found = 1;
            }
        }
    }

    if (!found) {
        printf("No record found with %s='%s' in table '%s'.\n", column_name, search_value, Table);
    }

    fclose(file);
}

void Delete(const char *Table, const char *column_name, const char *delete_value) {
    char filename[FILENAME_SIZE], temp_filename[FILENAME_SIZE];
    snprintf(filename, sizeof(filename), "%s.txt", Table);
    snprintf(temp_filename, sizeof(temp_filename), "%s_temp.txt", Table);

    FILE *file = fopen(filename, "r");
    FILE *temp_file = fopen(temp_filename, "w");

    if (!file || !temp_file) {
        perror("Error opening files");
        return;
    }

    char buffer[BUFFER_SIZE];
    int column_index = -1;
    int deleted = 0;

    // Copy schema and find column index
    fgets(buffer, sizeof(buffer), file);
    fputs(buffer, temp_file);
    char *schema = strchr(buffer, '[');
    if (schema) {
        char *token = strtok(schema, "[\",]");
        int index = 0;
        while (token) {
            trim_whitespace(token);
            if (strcmp(token, column_name) == 0) {
                column_index = index;
                break;
            }
            index++;
            token = strtok(NULL, "[\",]");
        }
    }

    if (column_index == -1) {
        printf("Column '%s' not found in table '%s'.\n", column_name, Table);
        fclose(file);
        fclose(temp_file);
        remove(temp_filename);
        return;
    }

    // Copy opening brace
    fgets(buffer, sizeof(buffer), file);
    fputs(buffer, temp_file);

    // Process records
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strstr(buffer, "}") && strlen(buffer) <= 3) {
            fputs(buffer, temp_file);
            break;
        }

        char *start = strchr(buffer, '{');
        char *end = strrchr(buffer, '}');
        if (start && end) {
            *end = '\0';
            start++;
            char *record[MAX_COLUMNS];
            int i = 0;
            char *token = strtok(start, "\",");
            while (token && i < MAX_COLUMNS) {
                trim_whitespace(token);
                record[i++] = token;
                token = strtok(NULL, "\",");
            }

            if (i > column_index && strcmp(record[column_index], delete_value) != 0) {
                fputs(buffer, temp_file);
                fputc('}', temp_file);
                if (strchr(end + 1, ',')) fputc(',', temp_file);
                fputc('\n', temp_file);
            } else {
                deleted++;
            }
        }
    }

    fclose(file);
    fclose(temp_file);

    remove(filename);
    rename(temp_filename, filename);

    printf("Deleted %d record(s) from Table '%s' where %s='%s'.\n", deleted, Table, column_name, delete_value);
}

void Update(const char *Table, const char *search_column, const char *search_value, const char *update_column, const char *new_value) {
    char filename[FILENAME_SIZE], temp_filename[FILENAME_SIZE];
    snprintf(filename, sizeof(filename), "%s.txt", Table);
    snprintf(temp_filename, sizeof(temp_filename), "%s_temp.txt", Table);

    FILE *file = fopen(filename, "r");
    FILE *temp_file = fopen(temp_filename, "w");

    if (!file || !temp_file) {
        perror("Error opening files");
        return;
    }

    char buffer[BUFFER_SIZE];
    int search_column_index = -1, update_column_index = -1;
    int updated = 0;

    // Copy schema and find column indices
    fgets(buffer, sizeof(buffer), file);
    fputs(buffer, temp_file);
    char *schema = strchr(buffer, '[');
    if (schema) {
        char *token = strtok(schema, "[\",]");
        int index = 0;
        while (token) {
            trim_whitespace(token);
            if (strcmp(token, search_column) == 0) {
                search_column_index = index;
            }
            if (strcmp(token, update_column) == 0) {
                update_column_index = index;
            }
            index++;
            token = strtok(NULL, "[\",]");
        }
    }

    if (search_column_index == -1) {
        printf("Search column '%s' not found in table '%s'.\n", search_column, Table);
        fclose(file);
        fclose(temp_file);
        remove(temp_filename);
        return;
    }

    if (update_column_index == -1) {
        printf("Update column '%s' not found in table '%s'.\n", update_column, Table);
        fclose(file);
        fclose(temp_file);
        remove(temp_filename);
        return;
    }

    // Copy opening brace
    fgets(buffer, sizeof(buffer), file);
    fputs(buffer, temp_file);

    // Process records
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strstr(buffer, "}") && strlen(buffer) <= 3) {
            fputs(buffer, temp_file);
            break;
        }

        char *start = strchr(buffer, '{');
        char *end = strrchr(buffer, '}');
        if (start && end) {
            *end = '\0';
            start++;
            char *record[MAX_COLUMNS];
            int i = 0;
            char *token = strtok(start, "\",");
            while (token && i < MAX_COLUMNS) {
                trim_whitespace(token);
                record[i++] = token;
                token = strtok(NULL, "\",");
            }

            if (i > search_column_index && strcmp(record[search_column_index], search_value) == 0) {
                record[update_column_index] = (char *)new_value; // update the value
                updated++;
            }
            fprintf(temp_file, "{");
            for (int j = 0; j < i; j++) {
                fprintf(temp_file, "\"%s\"", record[j]);
                if (j < i - 1) fprintf(temp_file, ",");
            }
            fprintf(temp_file, "},\n");
        }
    }

    fclose(file);
    fclose(temp_file);

    remove(filename);
    rename(temp_filename, filename);

    printf("Updated %d record(s) in Table '%s' where %s='%s' to %s='%s'.\n", updated, Table, search_column, search_value, update_column, new_value);
}

int main() {

    clock_t start_time = clock();

    const char *schema[] = {"name", "age", "city"};
    CreateTable("users", schema, 3);

    const char *data1[] = {"Alice", "30", "New York"};
    Insert("users", data1, 3);

    const char *data2[] = {"Bob", "25", "Los Angeles"};
    Insert("users", data2, 3);

    Get("users", "name", "Alice");
    Update("users", "name", "Alice", "age", "31");
    Delete("users", "name", "Bob");
    CloseTable("users");

    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %f seconds\n", elapsed_time);
    return 0;
}
