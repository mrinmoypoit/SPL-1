// Win count model : 
#include <stdio.h>
#include <string.h>

#define MAX_COMPONENTS 100
#define MAX_NAME_LEN 50

typedef struct {
    char name[MAX_NAME_LEN];
    int wins; // Count of wins
} Component;

void display_chart(Component components[], int n) {
    printf("\n--- Final Rankings ---\n");
    printf("Rank\tName\t\tWins\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%s\t\t%d\n", i + 1, components[i].name, components[i].wins);
    }
}

void rank_components(Component components[], int n) {
    // Sort components by wins in descending order (bubble sort)
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (components[j].wins < components[j + 1].wins) {
                // Swap components[j] and components[j+1]
                Component temp = components[j];
                components[j] = components[j + 1];
                components[j + 1] = temp;
            }
        }
    }
}

int main() {
    int num_components;

    printf("How many components are there? ");
    scanf("%d", &num_components);

    if (num_components < 2 || num_components > MAX_COMPONENTS) {
        printf("Invalid number of components. Exiting.\n");
        return 1;
    }

    Component components[MAX_COMPONENTS];

    // Step 1: Get the names of the components
    for (int i = 0; i < num_components; i++) {
        printf("Enter name of component %d: ", i + 1);
        scanf("%s", components[i].name);
        components[i].wins = 0; // Initialize win count
    }

    // Step 2: Pairwise comparison
    printf("\n--- Pairwise Comparisons ---\n");
    for (int i = 0; i < num_components; i++) {
        for (int j = i + 1; j < num_components; j++) {
            int choice;
            printf("Which is better? 1. %s or 2. %s: ", components[i].name, components[j].name);
            scanf("%d", &choice);

            if (choice == 1) {
                components[i].wins++;
            } else if (choice == 2) {
                components[j].wins++;
            } else {
                printf("Invalid choice. No points awarded.\n");
            }
        }
    }

    // Step 3: Rank components based on wins
    rank_components(components, num_components);

    // Step 4: Display the chart
    display_chart(components, num_components);

    return 0;
}
