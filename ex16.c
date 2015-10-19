#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct Person {
        char *name;
        int age;
        int height;
        int weight;
};

struct Person *Person_create(char *name, int age, int height, int weight)
{
        struct Person *who = malloc(sizeof(struct Person));
        assert(who != NULL);

        who->name = strdup(name);
        who->age = age;
        who->height = height;
        who->weight = weight;

        return who;
}

void Person_destroy(struct Person *who)
{
        assert(who != NULL);

        free(who->name);
        free(who);
}

void Person_print(struct Person *who)
{
        printf("Name: %s\n", who->name);
        printf("\tAge: %d\n", who->age);
        printf("\tHeight: %d\n", who->height);
        printf("\tWeight: %d\n", who->weight);
}

int main(int argc, char *argv[])
{
        struct Person *roman = Person_create(
                        "Roman Levin", 30, 175, 90);
        struct Person *alex = Person_create(
                        "Aleksandra Grochowska", 27, 175, 72);

        printf("Roman is at memory location %p:\n", roman);
        Person_print(roman);

        printf("Alex is at memory location %p:\n", alex);
        Person_print(alex);

        Person_destroy(roman);
        Person_destroy(alex);

        return 0;
}

