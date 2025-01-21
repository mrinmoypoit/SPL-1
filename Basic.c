#include <stdio.h>
#include <string.h>
#include <math.h>
#define MAX_COMPONENTS 100
#define MAX_NAME_LEN 50
#define K_FACTOR 32 // For Elo algorithm
#define PI 3.14159265358979 // For Glicko algorithm

typedef struct {
    char name[MAX_NAME_LEN];
    float wins;    // For Win rate algorithm
    float elo;     // For Elo algorithm
    double rating; // For Glicko algorithm
    double RD;     // For Glicko algorithm
} Component;

// Functions for Win rate algorithm
void display_chart_win_rate(Component components[], int n) {
    printf("\n--- Final Rankings (Win Rate) ---\n");
    printf("Rank\tName\t\tWins\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%s\t\t%.0f\n", i + 1, components[i].name, components[i].wins);
    }
}

void rank_components_win_rate(Component components[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (components[j].wins < components[j + 1].wins) {
                Component temp = components[j];
                components[j] = components[j + 1];
                components[j + 1] = temp;
            }
        }
    }
}

// Functions for Elo algorithm
float calculate_expected_score(float rating_a, float rating_b) {
    return 1.0 / (1.0 + pow(10.0, (rating_b - rating_a) / 400.0));
}

void update_elo_ratings(Component *winner, Component *loser) {
    float expected_winner = calculate_expected_score(winner->elo, loser->elo);
    float expected_loser = calculate_expected_score(loser->elo, winner->elo);

    winner->elo += K_FACTOR * (1.0 - expected_winner);
    loser->elo += K_FACTOR * (0.0 - expected_loser);
}

void display_chart_elo(Component components[], int n) {
    printf("\n--- Final Rankings (Elo) ---\n");
    printf("Rank\tName\t\tElo Rating\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%s\t\t%.2f\n", i + 1, components[i].name, components[i].elo);
    }
}

void rank_components_elo(Component components[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (components[j].elo < components[j + 1].elo) {
                Component temp = components[j];
                components[j] = components[j + 1];
                components[j + 1] = temp;
            }
        }
    }
}

// Functions for Glicko algorithm
double g(double RD) {
    return 1.0 / sqrt(1.0 + (3.0 * pow(RD, 2)) / (PI * PI));
}

double expected_score(double rating_a, double rating_b, double RD_b) {
    return 1.0 / (1.0 + pow(10.0, -(g(RD_b) * (rating_a - rating_b)) / 400.0));
}

void update_glicko_ratings(Component *winner, Component *loser) {
    const double q = log(10) / 400.0;

    double g_RD_loser = g(loser->RD);
    double g_RD_winner = g(winner->RD);

    double E_winner = expected_score(winner->rating, loser->rating, loser->RD);
    double E_loser = expected_score(loser->rating, winner->rating, winner->RD);

    double d2_winner = 1.0 / (q * q * g_RD_loser * g_RD_loser * E_winner * (1 - E_winner));
    double d2_loser = 1.0 / (q * q * g_RD_winner * g_RD_winner * E_loser * (1 - E_loser));

    winner->rating += (q / ((1.0 / (winner->RD * winner->RD)) + (1.0 / d2_winner))) *
                      g_RD_loser * (1 - E_winner);
    loser->rating += (q / ((1.0 / (loser->RD * loser->RD)) + (1.0 / d2_loser))) *
                     g_RD_winner * (0 - E_loser);

    winner->RD = sqrt(1.0 / ((1.0 / (winner->RD * winner->RD)) + (1.0 / d2_winner)));
    loser->RD = sqrt(1.0 / ((1.0 / (loser->RD * loser->RD)) + (1.0 / d2_loser)));
}

void display_chart_glicko(Component components[], int n) {
    printf("\n--- Final Rankings (Glicko) ---\n");
    printf("Rank\tName\t\tRating\t\tRD\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%s\t\t%.2f\t\t%.2f\n", i + 1, components[i].name, components[i].rating, components[i].RD);
    }
}

void rank_components_glicko(Component components[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (components[j].rating < components[j + 1].rating) {
                Component temp = components[j];
                components[j] = components[j + 1];
                components[j + 1] = temp;
            }
        }
    }
}

int main() {
    int num_components;
    int algorithm_choice;

    printf("Choose the algorithm: \n");
    printf("1. Win Rate\n");
    printf("2. Elo Rating\n");
    printf("3. Glicko Rating\n");
    printf("Enter your choice: ");
    scanf("%d", &algorithm_choice);

    printf("How many components are there? ");
    scanf("%d", &num_components);

    if (num_components < 2 || num_components > MAX_COMPONENTS) {
        printf("Invalid number of components. Exiting.\n");
        return 1;
    }

    Component components[MAX_COMPONENTS];

    for (int i = 0; i < num_components; i++) {
        printf("Enter name of component %d: ", i + 1);
        scanf("%s", components[i].name);
        components[i].wins = 0;
        components[i].elo = 1000.0;
        components[i].rating = 1500.0;
        components[i].RD = 350.0;
    }

    printf("\n--- Pairwise Comparisons ---\n");
    for (int i = 0; i < num_components; i++) {
        for (int j = i + 1; j < num_components; j++) {
            int choice;
            printf("Which is better? 1. %s or 2. %s: ", components[i].name, components[j].name);
            scanf("%d", &choice);

            if (algorithm_choice == 1) {
                if (choice == 1) {
                    components[i].wins++;
                } else if (choice == 2) {
                    components[j].wins++;
                } else {
                    printf("Invalid choice. No points awarded.\n");
                }
            } else if (algorithm_choice == 2) {
                if (choice == 1) {
                    update_elo_ratings(&components[i], &components[j]);
                } else if (choice == 2) {
                    update_elo_ratings(&components[j], &components[i]);
                } else {
                    printf("Invalid choice. No Elo update.\n");
                }
            } else if (algorithm_choice == 3) {
                if (choice == 1                    ) {
                    update_glicko_ratings(&components[i], &components[j]);
                } else if (choice == 2) {
                    update_glicko_ratings(&components[j], &components[i]);
                } else {
                    printf("Invalid choice. No Glicko update.\n");
                }
            } else {
                printf("Invalid algorithm choice. Exiting.\n");
                return 1;
            }
        }
    }

    if (algorithm_choice == 1) {
        rank_components_win_rate(components, num_components);
        display_chart_win_rate(components, num_components);
    } else if (algorithm_choice == 2) {
        rank_components_elo(components, num_components);
        display_chart_elo(components, num_components);
    } else if (algorithm_choice == 3) {
        rank_components_glicko(components, num_components);
        display_chart_glicko(components, num_components);
    } else {
        printf("Invalid algorithm choice. Exiting.\n");
        return 1;
    }

    return 0;
}
