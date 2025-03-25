#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define MAX_COMPONENTS 100
#define MAX_NAME_LEN 50
#define MAX_USERS 1000
#define MAX_VOTES 10000
#define K_FACTOR 32         // For Elo and Bradley-Terry algorithms
#define PI 3.14159265358979 // For Glicko algorithm
#define INITIAL_ELO 1000.0
#define INITIAL_RATING 1500.0
#define INITIAL_RD 350.0
#define INITIAL_MU 25.0     // For TrueSkill algorithm
#define INITIAL_SIGMA 8.333 // For TrueSkill algorithm
#define DAMPING_FACTOR 0.85 // For PageRank algorithm

typedef struct
{
    char name[MAX_NAME_LEN];
    float wins;            // For Win rate algorithm
    float elo;             // For Elo algorithm
    double rating;         // For Glicko and Bradley-Terry algorithms
    double RD;             // For Glicko algorithm
    double mu;             // For TrueSkill algorithm
    double sigma;          // For TrueSkill algorithm
    double pagerank;       // For PageRank algorithm
    double bayesian_score; // For Bayesian ranking
} Component;

typedef struct
{
    int user_id;
    char topic[MAX_NAME_LEN];
    char user_name[MAX_NAME_LEN];
    time_t timestamp;
    Component components[MAX_COMPONENTS];
    int num_components;
    int algorithm_choice;
    char share_code[10];                       // Unique code for sharing comparisons
    int votes[MAX_COMPONENTS][MAX_COMPONENTS]; // Voting matrix
} UserComparison;

// Function prototypes
void display_chart_win_rate(Component components[], int n);
void rank_components_win_rate(Component components[], int n);
float calculate_expected_score(float rating_a, float rating_b);
void update_elo_ratings(Component *winner, Component *loser);
void display_chart_elo(Component components[], int n);
void rank_components_elo(Component components[], int n);
double g(double RD);
double expected_score(double rating_a, double rating_b, double RD_b);
void update_glicko_ratings(Component *winner, Component *loser);
void display_chart_glicko(Component components[], int n);
void rank_components_glicko(Component components[], int n);
void rank_components(Component components[], int n, int (*compare)(Component, Component));
int compare_win_rate(Component a, Component b);
int compare_elo(Component a, Component b);
int compare_glicko(Component a, Component b);
double calculate_bradley_terry_score(double rating_a, double rating_b);
void update_bradley_terry_ratings(Component *winner, Component *loser);
void display_chart_bradley_terry(Component components[], int n);
void rank_components_bradley_terry(Component components[], int n);
void update_trueskill_ratings(Component *winner, Component *loser);
void display_chart_trueskill(Component components[], int n);
void rank_components_trueskill(Component components[], int n);
int compare_trueskill(Component a, Component b);
int load_votes_from_file(const char *filename, UserComparison *user_comparison);
void save_votes_to_file(const char *filename, UserComparison *user_comparison);
void display_previous_comparisons();
int generate_user_id();
void generate_share_code(char *code);
void add_vote(UserComparison *user_comparison, int component_a, int component_b, int vote);
void aggregate_votes(UserComparison *user_comparison);
void calculate_pagerank(Component components[], int n, int votes[MAX_COMPONENTS][MAX_COMPONENTS]);
void calculate_bayesian_ranking(Component components[], int n);
void generate_and_save_user_id(const char *user_name);
void save_user_data(int user_id, const UserComparison *user_comparison);
void process_votes_and_update_ratings(UserComparison *user_comparison);

// Global variables
UserComparison users[MAX_USERS];
int num_users = 0;

// Functions for Win rate algorithm
void display_chart_win_rate(Component components[], int n)
{
    printf("\n--- Final Rankings (Win Rate) ---\n");
    printf("Rank\tName\t\tWins\n");
    for (int i = 0; i < n; i++)
    {
        printf("%d\t%s\t\t%.0f\n", i + 1, components[i].name, components[i].wins);
    }
}

void rank_components_win_rate(Component components[], int n)
{
    rank_components(components, n, compare_win_rate);
}

// Functions for Elo algorithm
float calculate_expected_score(float rating_a, float rating_b)
{
    return 1.0 / (1.0 + pow(10.0, (rating_b - rating_a) / 400.0));
}

void update_elo_ratings(Component *winner, Component *loser)
{
    float expected_winner = calculate_expected_score(winner->elo, loser->elo);
    float expected_loser = calculate_expected_score(loser->elo, winner->elo);

    winner->elo += K_FACTOR * (1.0 - expected_winner);
    loser->elo += K_FACTOR * (0.0 - expected_loser);
}

void display_chart_elo(Component components[], int n)
{
    printf("\n--- Final Rankings (Elo) ---\n");
    printf("Rank\tName\t\tElo Rating\n");
    for (int i = 0; i < n; i++)
    {
        printf("%d\t%s\t\t%.2f\n", i + 1, components[i].name, components[i].elo);
    }
}

void rank_components_elo(Component components[], int n)
{
    rank_components(components, n, compare_elo);
}

// Functions for Glicko algorithm
double g(double RD)
{
    return 1.0 / sqrt(1.0 + (3.0 * pow(RD, 2)) / (PI * PI));
}

double expected_score(double rating_a, double rating_b, double RD_b)
{
    return 1.0 / (1.0 + pow(10.0, -(g(RD_b) * (rating_a - rating_b)) / 400.0));
}

void update_glicko_ratings(Component *winner, Component *loser)
{
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

void display_chart_glicko(Component components[], int n)
{
    printf("\n--- Final Rankings (Glicko) ---\n");
    printf("Rank\tName\t\tRating\t\tRD\n");
    for (int i = 0; i < n; i++)
    {
        printf("%d\t%s\t\t%.2f\t\t%.2f\n", i + 1, components[i].name, components[i].rating, components[i].RD);
    }
}

void rank_components_glicko(Component components[], int n)
{
    rank_components(components, n, compare_glicko);
}

// Functions for Bradley-Terry model
double calculate_bradley_terry_score(double rating_a, double rating_b)
{
    return rating_a / (rating_a + rating_b);
}

void update_bradley_terry_ratings(Component *winner, Component *loser)
{
    double winner_score = calculate_bradley_terry_score(winner->rating, loser->rating);
    double loser_score = calculate_bradley_terry_score(loser->rating, winner->rating);

    winner->rating += K_FACTOR * (1.0 - winner_score);
    loser->rating += K_FACTOR * (0.0 - loser_score);
}

void display_chart_bradley_terry(Component components[], int n)
{
    printf("\n--- Final Rankings (Bradley-Terry) ---\n");
    printf("Rank\tName\t\tRating\n");
    for (int i = 0; i < n; i++)
    {
        printf("%d\t%s\t\t%.2f\n", i + 1, components[i].name, components[i].rating);
    }
}

void rank_components_bradley_terry(Component components[], int n)
{
    rank_components(components, n, compare_glicko); // Using the same comparison as Glicko
}

// Functions for TrueSkill algorithm
void update_trueskill_ratings(Component *winner, Component *loser)
{
    const double beta = 4.166; // Skill variance
    const double tau = 0.083;  // Dynamic factor
    const double draw_probability = 0.1;

    double c = sqrt(2 * beta * beta + winner->sigma * winner->sigma + loser->sigma * loser->sigma);
    double expected_winner = 1.0 / (1.0 + exp((loser->mu - winner->mu) / c));
    double expected_loser = 1.0 - expected_winner;

    double winner_update = (winner->sigma * winner->sigma) / c * (1 - expected_winner);
    double loser_update = (loser->sigma * loser->sigma) / c * (0 - expected_loser);

    winner->mu += winner_update;
    loser->mu += loser_update;

    winner->sigma = sqrt(winner->sigma * winner->sigma + tau * tau);
    loser->sigma = sqrt(loser->sigma * loser->sigma + tau * tau);
}

void display_chart_trueskill(Component components[], int n)
{
    printf("\n--- Final Rankings (TrueSkill) ---\n");
    printf("Rank\tName\t\tMu\t\tSigma\n");
    for (int i = 0; i < n; i++)
    {
        printf("%d\t%s\t\t%.2f\t\t%.2f\n", i + 1, components[i].name, components[i].mu, components[i].sigma);
    }
}

void rank_components_trueskill(Component components[], int n)
{
    rank_components(components, n, compare_trueskill);
}

// Generic ranking function
void rank_components(Component components[], int n, int (*compare)(Component, Component))
{
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (compare(components[j], components[j + 1]) < 0)
            {
                Component temp = components[j];
                components[j] = components[j + 1];
                components[j + 1] = temp;
            }
        }
    }
}

// Comparison functions
int compare_win_rate(Component a, Component b)
{
    return (a.wins > b.wins) - (a.wins < b.wins);
}

int compare_elo(Component a, Component b)
{
    return (a.elo > b.elo) - (a.elo < b.elo);
}

int compare_glicko(Component a, Component b)
{
    return (a.rating > b.rating) - (a.rating < b.rating);
}

int compare_trueskill(Component a, Component b)
{
    return (a.mu > b.mu) - (a.mu < b.mu);
}

// Load votes from file
int load_votes_from_file(const char *filename, UserComparison *user_comparison)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("No saved votes found.\n");
        return 0;
    }

    fscanf(file, "%d", &user_comparison->user_id);
    fscanf(file, "%s", user_comparison->topic);
    fscanf(file, "%s", user_comparison->user_name);
    fscanf(file, "%ld", &user_comparison->timestamp);
    fscanf(file, "%d", &user_comparison->num_components);
    fscanf(file, "%d", &user_comparison->algorithm_choice);
    fscanf(file, "%s", user_comparison->share_code);

    for (int i = 0; i < user_comparison->num_components; i++)
    {
        fscanf(file, "%s %f %f %lf %lf %lf %lf %lf %lf", user_comparison->components[i].name, 
              &user_comparison->components[i].wins, &user_comparison->components[i].elo, 
              &user_comparison->components[i].rating, &user_comparison->components[i].RD, 
              &user_comparison->components[i].mu, &user_comparison->components[i].sigma, 
              &user_comparison->components[i].pagerank, &user_comparison->components[i].bayesian_score);
    }

    for (int i = 0; i < user_comparison->num_components; i++)
    {
        for (int j = 0; j < user_comparison->num_components; j++)
        {
            fscanf(file, "%d", &user_comparison->votes[i][j]);
        }
    }

    fclose(file);
    return 1;
}

// Save votes to file
void save_votes_to_file(const char *filename, UserComparison *user_comparison)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Error opening file to save votes.\n");
        return;
    }

    fprintf(file, "%d\n", user_comparison->user_id);
    fprintf(file, "%s\n", user_comparison->topic);
    fprintf(file, "%s\n", user_comparison->user_name);
    fprintf(file, "%ld\n", user_comparison->timestamp);
    fprintf(file, "%d\n", user_comparison->num_components);
    fprintf(file, "%d\n", user_comparison->algorithm_choice);
    fprintf(file, "%s\n", user_comparison->share_code);

    for (int i = 0; i < user_comparison->num_components; i++)
    {
        fprintf(file, "%s %.0f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", 
               user_comparison->components[i].name, 
               user_comparison->components[i].wins, 
               user_comparison->components[i].elo, 
               user_comparison->components[i].rating, 
               user_comparison->components[i].RD, 
               user_comparison->components[i].mu, 
               user_comparison->components[i].sigma, 
               user_comparison->components[i].pagerank, 
               user_comparison->components[i].bayesian_score);
    }

    for (int i = 0; i < user_comparison->num_components; i++)
    {
        for (int j = 0; j < user_comparison->num_components; j++)
        {
            fprintf(file, "%d ", user_comparison->votes[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// Display previous comparisons
void display_previous_comparisons()
{
    FILE *file = fopen("User_id_history.txt", "r");
    if (file == NULL)
    {
        printf("No previous comparisons found.\n");
        return;
    }

    printf("\n--- Previous Comparisons ---\n");
    printf("User ID\tUser Name\tTimestamp\n");
    int user_id;
    char user_name[MAX_NAME_LEN];
    time_t timestamp;
    while (fscanf(file, "%d %s %ld", &user_id, user_name, &timestamp) != EOF)
    {
        printf("%03d\t%s\t\t%s", user_id, user_name, ctime(&timestamp));
    }
    fclose(file);
}

// Generate a new user ID
int generate_user_id()
{
    static int user_id = 0;
    return ++user_id;
}

// Generate a unique share code
void generate_share_code(char *code)
{
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < 9; i++)
    {
        code[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    code[9] = '\0';
}

// Add a vote to the voting matrix
void add_vote(UserComparison *user_comparison, int component_a, int component_b, int vote)
{
    user_comparison->votes[component_a][component_b] += vote;
}

// Aggregate votes to generate cumulative rankings
void aggregate_votes(UserComparison *user_comparison)
{
    for (int i = 0; i < user_comparison->num_components; i++)
    {
        user_comparison->components[i].wins = 0;
        for (int j = 0; j < user_comparison->num_components; j++)
        {
            user_comparison->components[i].wins += user_comparison->votes[i][j];
        }
    }
}

// Calculate PageRank for components
void calculate_pagerank(Component components[], int n, int votes[MAX_COMPONENTS][MAX_COMPONENTS])
{
    double initial_rank = 1.0 / n;
    for (int i = 0; i < n; i++)
    {
        components[i].pagerank = initial_rank;
    }

    double new_ranks[MAX_COMPONENTS];
    for (int iter = 0; iter < 100; iter++)
    {
        for (int i = 0; i < n; i++)
        {
            new_ranks[i] = (1 - DAMPING_FACTOR) / n;
            for (int j = 0; j < n; j++)
            {
                if (votes[j][i] > 0)
                {
                    new_ranks[i] += DAMPING_FACTOR * components[j].pagerank / (double)votes[j][i];
                }
            }
        }

        for (int i = 0; i < n; i++)
        {
            components[i].pagerank = new_ranks[i];
        }
    }
}

// Calculate Bayesian ranking for components
void calculate_bayesian_ranking(Component components[], int n)
{
    for (int i = 0; i < n; i++)
    {
        components[i].bayesian_score = components[i].wins / (components[i].wins + 1.0);
    }
}

// Generate and save user ID
void generate_and_save_user_id(const char *user_name)
{
    int user_id = generate_user_id();

    // Save the user ID, user name, and timestamp to "User_id_history.txt"
    FILE *user_id_file = fopen("User_id_history.txt", "a"); // Open in append mode
    if (user_id_file == NULL)
    {
        printf("Error opening User_id_history.txt file.\n");
        return;
    }
    time_t timestamp = time(NULL);
    fprintf(user_id_file, "%d %s %ld\n", user_id, user_name, timestamp);
    fclose(user_id_file);

    // Create a file named after the user ID to store user data
    char filename[20];
    sprintf(filename, "%d.txt", user_id);
    FILE *user_file = fopen(filename, "w");
    if (user_file == NULL)
    {
        printf("Error creating user data file.\n");
        return;
    }
    fclose(user_file);

    printf("User ID %d generated and saved for user: %s\n", user_id, user_name);
}

// Save user data to file
void save_user_data(int user_id, const UserComparison *user_comparison)
{
    char filename[20];
    sprintf(filename, "%d.txt", user_id);

    // Open the file in append mode to preserve existing data
    FILE *user_file = fopen(filename, "a");
    if (user_file == NULL)
    {
        printf("Error opening user data file.\n");
        return;
    }

    // Save the user comparison data
    fprintf(user_file, "--- User Comparison Data ---\n");
    fprintf(user_file, "User ID: %d\n", user_comparison->user_id);
    fprintf(user_file, "Topic: %s\n", user_comparison->topic);
    fprintf(user_file, "User Name: %s\n", user_comparison->user_name);
    fprintf(user_file, "Timestamp: %ld\n", user_comparison->timestamp);
    fprintf(user_file, "Algorithm Choice: %d\n", user_comparison->algorithm_choice);
    fprintf(user_file, "Share Code: %s\n", user_comparison->share_code);

    // Save component data
    fprintf(user_file, "\n--- Components ---\n");
    for (int i = 0; i < user_comparison->num_components; i++)
    {
        fprintf(user_file, "Component %d: %s\n", i + 1, user_comparison->components[i].name);
        fprintf(user_file, "Wins: %.0f, Elo: %.2f, Rating: %.2f, RD: %.2f, Mu: %.2f, Sigma: %.2f, PageRank: %.4f, Bayesian Score: %.4f\n",
                user_comparison->components[i].wins,
                user_comparison->components[i].elo,
                user_comparison->components[i].rating,
                user_comparison->components[i].RD,
                user_comparison->components[i].mu,
                user_comparison->components[i].sigma,
                user_comparison->components[i].pagerank,
                user_comparison->components[i].bayesian_score);
    }

    // Save voting matrix
    fprintf(user_file, "\n--- Voting Matrix ---\n");
    for (int i = 0; i < user_comparison->num_components; i++)
    {
        for (int j = 0; j < user_comparison->num_components; j++)
        {
            fprintf(user_file, "%d ", user_comparison->votes[i][j]);
        }
        fprintf(user_file, "\n");
    }

    fclose(user_file);
    printf("User data saved to %s.\n", filename);
}

// Process votes and update ratings based on the chosen algorithm
void process_votes_and_update_ratings(UserComparison *user_comparison)
{
    for (int i = 0; i < user_comparison->num_components; i++)
    {
        for (int j = 0; j < user_comparison->num_components; j++)
        {
            if (user_comparison->votes[i][j] > 0)
            {
                // For each win in the voting matrix, update the ratings
                for (int k = 0; k < user_comparison->votes[i][j]; k++)
                {
                    if (user_comparison->algorithm_choice == 1)
                    {
                        user_comparison->components[i].wins++;
                    }
                    else if (user_comparison->algorithm_choice == 2)
                    {
                        update_elo_ratings(&user_comparison->components[i], &user_comparison->components[j]);
                    }
                    else if (user_comparison->algorithm_choice == 3)
                    {
                        update_glicko_ratings(&user_comparison->components[i], &user_comparison->components[j]);
                    }
                    else if (user_comparison->algorithm_choice == 4)
                    {
                        update_bradley_terry_ratings(&user_comparison->components[i], &user_comparison->components[j]);
                    }
                    else if (user_comparison->algorithm_choice == 5)
                    {
                        update_trueskill_ratings(&user_comparison->components[i], &user_comparison->components[j]);
                    }
                }
            }
        }
    }
}

int main()
{
    srand(time(NULL)); // Seed for random share code generation

    int choice;
    printf("Do you want to use previous comparisons or start a new one? (1 for Previous, 2 for New): ");
    if (scanf("%d", &choice) != 1)
    {
        printf("Invalid input. Exiting.\n");
        return 1;
    }

    UserComparison user_comparison;
    if (choice == 1)
    {
        display_previous_comparisons();
        printf("Enter the User ID to load: ");
        int user_id;
        if (scanf("%d", &user_id) != 1)
        {
            printf("Invalid input. Exiting.\n");
            return 1;
        }

        char filename[20];
        sprintf(filename, "%d.txt", user_id);
        if (!load_votes_from_file(filename, &user_comparison))
        {
            printf("Failed to load comparison. Exiting.\n");
            return 1;
        }
    }
    else if (choice == 2)
    {
        user_comparison.user_id = generate_user_id();
        printf("New User ID: %03d\n", user_comparison.user_id);

        printf("Enter the comparison topic: ");
        scanf("%s", user_comparison.topic);
        printf("Enter your name: ");
        scanf("%s", user_comparison.user_name);
        user_comparison.timestamp = time(NULL);

        generate_share_code(user_comparison.share_code);
        printf("Share Code: %s\n", user_comparison.share_code);

        printf("Choose the algorithm: \n");
        printf("1. Win Rate\n");
        printf("2. Elo Rating\n");
        printf("3. Glicko Rating\n");
        printf("4. Bradley-Terry Rating\n");
        printf("5. TrueSkill Rating\n");
        printf("6. PageRank\n");
        printf("7. Bayesian Ranking\n");
        printf("Enter your choice: ");
        if (scanf("%d", &user_comparison.algorithm_choice) != 1)
        {
            printf("Invalid input. Exiting.\n");
            return 1;
        }

        printf("How many components are there? ");
        if (scanf("%d", &user_comparison.num_components) != 1 || 
            user_comparison.num_components < 2 || 
            user_comparison.num_components > MAX_COMPONENTS)
        {
            printf("Invalid number of components. Exiting.\n");
            return 1;
        }

        for (int i = 0; i < user_comparison.num_components; i++)
        {
            printf("Enter name of component %d: ", i + 1);
            if (scanf("%49s", user_comparison.components[i].name) != 1)
            {
                printf("Invalid input. Exiting.\n");
                return 1;
            }
            user_comparison.components[i].wins = 0;
            user_comparison.components[i].elo = INITIAL_ELO;
            user_comparison.components[i].rating = INITIAL_RATING;
            user_comparison.components[i].RD = INITIAL_RD;
            user_comparison.components[i].mu = INITIAL_MU;
            user_comparison.components[i].sigma = INITIAL_SIGMA;
            user_comparison.components[i].pagerank = 0.0;
            user_comparison.components[i].bayesian_score = 0.0;
        }

        // Initialize voting matrix
        for (int i = 0; i < user_comparison.num_components; i++)
        {
            for (int j = 0; j < user_comparison.num_components; j++)
            {
                user_comparison.votes[i][j] = 0;
            }
        }
    }
    else
    {
        printf("Invalid choice. Exiting.\n");
        return 1;
    }

    printf("\n--- Pairwise Comparisons ---\n");
    for (int i = 0; i < user_comparison.num_components; i++)
    {
        for (int j = i + 1; j < user_comparison.num_components; j++)
        {
            int choice;
            printf("Which is better? 1. %s or 2. %s", 
                  user_comparison.components[i].name, 
                  user_comparison.components[j].name);
            if (user_comparison.algorithm_choice == 4)
            {
                printf(" (0 to skip): ");
            }
            else
            {
                printf(": ");
            }
            if (scanf("%d", &choice) != 1)
            {
                printf("Invalid input. Exiting.\n");
                return 1;
            }

            if (choice == 1)
            {
                add_vote(&user_comparison, i, j, 1);
            }
            else if (choice == 2)
            {
                add_vote(&user_comparison, j, i, 1);
            }
            else if (choice == 0 && user_comparison.algorithm_choice == 4)
            {
                // Skip this comparison
            }
            else
            {
                printf("Invalid choice. No update.\n");
            }
        }
    }

    // Process the votes and update ratings
    process_votes_and_update_ratings(&user_comparison);

    // Aggregate votes (for win rate, PageRank, and Bayesian)
    aggregate_votes(&user_comparison);

    // Calculate rankings based on the chosen algorithm
    if (user_comparison.algorithm_choice == 1)
    {
        rank_components_win_rate(user_comparison.components, user_comparison.num_components);
        display_chart_win_rate(user_comparison.components, user_comparison.num_components);
    }
    else if (user_comparison.algorithm_choice == 2)
    {
        rank_components_elo(user_comparison.components, user_comparison.num_components);
        display_chart_elo(user_comparison.components, user_comparison.num_components);
    }
    else if (user_comparison.algorithm_choice == 3)
    {
        rank_components_glicko(user_comparison.components, user_comparison.num_components);
        display_chart_glicko(user_comparison.components, user_comparison.num_components);
    }
    else if (user_comparison.algorithm_choice == 4)
    {
        rank_components_bradley_terry(user_comparison.components, user_comparison.num_components);
        display_chart_bradley_terry(user_comparison.components, user_comparison.num_components);
    }
    else if (user_comparison.algorithm_choice == 5)
    {
        rank_components_trueskill(user_comparison.components, user_comparison.num_components);
        display_chart_trueskill(user_comparison.components, user_comparison.num_components);
    }
    else if (user_comparison.algorithm_choice == 6)
    {
        calculate_pagerank(user_comparison.components, user_comparison.num_components, user_comparison.votes);
        printf("\n--- Final Rankings (PageRank) ---\n");
        printf("Rank\tName\t\tPageRank\n");
        for (int i = 0; i < user_comparison.num_components; i++)
        {
            printf("%d\t%s\t\t%.4f\n", i + 1, user_comparison.components[i].name, user_comparison.components[i].pagerank);
        }
    }
    else if (user_comparison.algorithm_choice == 7)
    {
        calculate_bayesian_ranking(user_comparison.components, user_comparison.num_components);
        printf("\n--- Final Rankings (Bayesian) ---\n");
        printf("Rank\tName\t\tBayesian Score\n");
        for (int i = 0; i < user_comparison.num_components; i++)
        {
            printf("%d\t%s\t\t%.4f\n", i + 1, user_comparison.components[i].name, user_comparison.components[i].bayesian_score);
        }
    }
    else
    {
        printf("Invalid algorithm choice. Exiting.\n");
        return 1;
    }

    // Save the final rankings to the file
    char filename[20];
    sprintf(filename, "%d.txt", user_comparison.user_id);
    save_votes_to_file(filename, &user_comparison);
    printf("Final rankings saved to %s.\n", filename);

    // Save user data
    save_user_data(user_comparison.user_id, &user_comparison);

    return 0;
}
