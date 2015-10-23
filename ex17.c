#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_DATA 512
#define MAX_ROWS 100

struct Row {
    int id;
    char *name;
    char *email;
};

struct Database {
    struct Row **rows;
    int max_data;
    int max_rows;
    int current_rows;
};

struct Connection {
    FILE *file;
    struct Database *db;
};

struct FileHeader {
    int max_data;
    int max_rows;
    int current_rows;
};

struct RowHeader {
    int id;
    int name_size;
    int email_size;
};

void Database_close(struct Connection *conn);

void die(const char *message, struct Connection *conn) {
    if (errno) {
        perror(message);
    } else {
        printf("ERROR: %s\n", message);
    }

    Database_close(conn);

    exit(1);
}

void Row_close(struct Row *row) {
    if (row) {
        free(row->email);
        free(row->name);
        free(row);
    }
}

struct Row *Row_create(int id, const char *name, const char *email, int max_data) {
    struct Row *row = malloc(sizeof(struct Row));
    row->id = id;
    row->name = strndup(name, max_data * sizeof(char));
    row->email = strndup(email, max_data * sizeof(char));
    if (!(row && row->name && row->email)) {
        Row_close(row);
        row = NULL;
    } else {
        // ensure that fields are null byte terminated
        // row->name[max_data - 1] = '\0';
        // row->email[max_data - 1] = '\0';
    }
    return row;
}

void Row_print(struct Row *row) { printf("%d %s %s\n", row->id, row->name, row->email); }

void Row_serialize(struct Row *row, struct Connection *conn) {
    struct RowHeader row_header = {.id = row->id, .name_size = strlen(row->name), .email_size = strlen(row->email)};

    int rc = fwrite(&row_header, sizeof(struct RowHeader), 1, conn->file);
    if (rc != 1)
        die("Failed to write row header.", conn);

    rc = fputs(row->name, conn->file);
    if (rc != 1)
        die("Failed to write name to file.", conn);

    rc = fputs(row->email, conn->file);
    if (rc != 1)
        die("Failed to write email to file.", conn);
}

void Row_deserialize(struct Connection *conn) {
    struct RowHeader row_header;

    int rc = fread(&row_header, sizeof(struct RowHeader), 1, conn->file);
    if (rc != 1)
        die("Failed to load row header", conn);

    char *name = calloc(row_header.name_size, sizeof(char));
    char *rc_fgets = fgets(name, row_header.name_size + 1, conn->file);
    if (!rc_fgets)
        die("Failed to load name field", conn);

    char *email = calloc(row_header.email_size, sizeof(char));
    rc_fgets = fgets(email, row_header.email_size + 1, conn->file);
    if (!rc_fgets)
        die("Failed to load email field", conn);

    conn->db->rows[row_header.id] = Row_create(row_header.id, name, email, conn->db->max_data);
    if (!conn->db->rows[row_header.id])
        die("Failed to created deserialized row", conn);
}

void Database_create(struct Connection *conn, int max_data, int max_rows);


void Database_load(struct Connection *conn) {
    struct FileHeader file_header;
    int rc = fread(&file_header, sizeof(struct FileHeader), 1, conn->file);
    if (rc != 1)
        die("Failed to load FileHeader.", conn);
    Database_create(conn, file_header.max_data, file_header.max_rows);
    for (int i = 0; i < file_header.current_rows; i++) {
        Row_deserialize(conn);
    }
}

void Database_write(struct Connection *conn) {
    rewind(conn->file);

    struct FileHeader file_header = {
        .max_rows = conn->db->max_rows, .max_data = conn->db->max_data, .current_rows = conn->db->current_rows};

    int rc = fwrite(&file_header, sizeof(struct FileHeader), 1, conn->file);
    if (rc != 1)
        die("Failed to write file header.", conn);

    for(int i = 0; i < file_header.max_rows; i++) {
        if (!conn->db->rows[i])
            continue;
        Row_serialize(conn->db->rows[i], conn);
    }

    rc = fflush(conn->file);
    if (rc == -1)
        die("Cannot flush database.", conn);
}

struct Connection *Database_open(const char *filename, char mode, int max_data, int max_rows) {
    struct Connection *conn = malloc(sizeof(struct Connection));
    if (!conn)
        die("Memory error", conn);

    conn->db = malloc(sizeof(struct Database));
    if (!conn->db)
        die("Memory error", conn);

    if (mode == 'c') {
        conn->file = fopen(filename, "w");
        Database_create(conn, max_data, max_rows);
    } else {
        conn->file = fopen(filename, "r+");

        if (conn->file) {
            Database_load(conn);
        }
    }

    if (!conn->file)
        die("Failed to open the file", conn);

    return conn;
}

void Database_close(struct Connection *conn) {
    if (conn) {
        if (conn->file) {
            int rc = fclose(conn->file);
            if (rc)
                printf("Failed to close file: %s\n", strerror(errno));
        }
        if (conn->db) {
            for (int i = 0; i < conn->db->max_rows; i++) {
                Row_close(conn->db->rows[i]);
            }
            free(conn->db->rows);
            free(conn->db);
        }
        free(conn);
    }
}

void Database_create(struct Connection *conn, int max_data, int max_rows) {
    conn->db->max_data = max_data;
    conn->db->max_rows = max_rows;
    conn->db->rows = calloc(max_rows, sizeof(struct Address *));
    conn->db->current_rows = 0;
}

void Database_set(struct Connection *conn, int id, const char *name, const char *email) {
    struct Row *row = conn->db->rows[id];
    if (row)
        die("Already set, delete it first", conn);

    row = Row_create(id, name, email, conn->db->max_data);
    if (!row)
        die("Cannot create row", conn);

    conn->db->rows[id] = row;
    conn->db->current_rows++;
}

void Database_get(struct Connection *conn, int id) {
    struct Row *row = conn->db->rows[id];

    if (row) {
        Row_print(row);
    } else {
        die("Row is not set", conn);
    }
}

void Database_delete(struct Connection *conn, int id) {
    if(conn->db->rows[id]){
        free(conn->db->rows[id]);
        conn->db->rows[id] = NULL;
        conn->db->current_rows--;
    }
}

void Database_list(struct Connection *conn) {
    struct Database *db = conn->db;

    for (int i = 0; i < MAX_ROWS; i++) {
        struct Row *cur = db->rows[i];

        if (cur) {
            Row_print(cur);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3)
        die("USAGE: ex17 <dbfile> <action> [action params]", NULL);

    char *filename = argv[1];
    char action = argv[2][0];
    struct Connection *conn;
    int id = 0;

    if (action == 'c') {
        int max_data = 0, max_rows = 0;

        if (argc == 3) {
            max_data = MAX_DATA;
            max_rows = MAX_ROWS;
        } else if (argc == 5) {
            max_data = atoi(argv[3]);
            max_rows = atoi(argv[4]);
        } else
            die("USAGE: ex <dbfile> c [max_data max_rows]", NULL);
        conn = Database_open(filename, action, max_data, max_rows);
        Database_write(conn);
    } else {
        conn = Database_open(filename, action, 0, 0);
    }


    if (argc > 3)
        id = atoi(argv[3]);
    if (id >= conn->db->max_rows)
        die("There's not that many records.", conn);

    switch (action) {
    case 'c':
        break;
    case 'g':
        if (argc != 4)
            die("Need an id to get", conn);

        Database_get(conn, id);
        break;

    case 's':
        if (argc != 6)
            die("Need id, name, email to set", conn);

        Database_set(conn, id, argv[4], argv[5]);
        Database_write(conn);
        break;

    case 'd':
        if (argc != 4)
            die("need id to delete", conn);

        Database_delete(conn, id);
        Database_write(conn);
        break;

    case 'l':
        Database_list(conn);
        break;

    default:
        die("Invalid action, only: c=create, g=get, s=set, d=del, l=list", conn);
    }

    Database_close(conn);

    return 0;
}
