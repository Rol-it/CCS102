#include <stdio.h>
#include <string.h>

#define MAX_USERNAME 50
#define MAX_EMAIL 100
#define MAX_PASSWORD 50
#define NUM_CONTESTANTS 4
#define NUM_JUDGES 3
#define NUM_CRITERIA 4

// ---------- Criterion definitions ----------
char criterionNames[NUM_CRITERIA][20] = {"Performance", "Creativity", "Originality", "Overall Impact"};
float criterionWeights[NUM_CRITERIA] = {0.40, 0.30, 0.20, 0.10};

// ---------- Account data ----------
char adminUsername[MAX_USERNAME] = "admin";
char adminEmail[MAX_EMAIL]       = "admin@judging.com";
char adminPassword[MAX_PASSWORD] = "admin123";

char judgeUsernames[NUM_JUDGES][MAX_USERNAME] = {"judge1", "judge2", "judge3"};
char judgeEmails[NUM_JUDGES][MAX_EMAIL]       = {"judge1@judging.com", "judge2@judging.com", "judge3@judging.com"};
char judgePasswords[NUM_JUDGES][MAX_PASSWORD] = {"pass1", "pass2", "pass3"};

// ---------- Account lock for role segregation ----------
// When a user selects a specific account at the main menu (Admin or a specific Judge),
// this lock prevents them from using any other account's credentials.
// 0 = open (any account allowed), 1 = Admin only, 2/3/4 = Judge 1/2/3 only
int accountLock = 0;

// ---------- Judge scores: judgeScores[judge][criterion][contestant] ----------
float judgeScores[NUM_JUDGES][NUM_CRITERIA][NUM_CONTESTANTS] = {0};

// ---------- Prototypes ----------
void printMainMenu();
void printJudgeMenu();
void printAdminMenu();
void printRubricsText();
void login(int *role, int lockedRole);
void runJudgeMenu(int judgeNum);
void runAdminMenu();
void viewRubrics();
void scoreContestants(int judgeNum);
void viewMyScoringSheet(int judgeNum);
void changeUsername(int roleType);
void changePassword(int roleType);
void viewOverallScoringSheet();
void viewContestantRankings();
void displayJudgeAccounts();
char* getRoleName(int role);
float computeFinalScore(float perf, float creativity, float originality, float impact);
float getJudgeCriterionScore(int judge, int c, int criterion);

int main() {
    int choice;
    int role = 0; // 0 = no active login, 1 = Admin, 2/3/4 = Judge 1/2/3
    int scanResult;

    while (1) {
        if (role == 0) {
            // Main menu (only when no one is logged in)
            printMainMenu();
            printf("\nEnter your choice: ");
            scanResult = scanf(" %d", &choice);

            if (scanResult == EOF) {
                printf("\nGoodbye!\n");
                return 0;
            }

            // Lock the account choice permanently - user cannot switch roles
            accountLock = choice;

            switch (choice) {
                case 1: { // Admin
                    int r = 1;
                    login(&r, accountLock);
                    role = r;
                    break;
                }
                case 2: { // Judge 1
                    int r = 2;
                    login(&r, accountLock);
                    role = r;
                    break;
                }
                case 3: { // Judge 2
                    int r = 3;
                    login(&r, accountLock);
                    role = r;
                    break;
                }
                case 4: { // Judge 3
                    int r = 4;
                    login(&r, accountLock);
                    role = r;
                    break;
                }
                case 5: // Exit
                    printf("\nGoodbye!\n");
                    return 0;
                default:
                    printf("\nInvalid choice. Please try again.\n");
            }
        } else {
            // Role-driven menu loop
            // If the user is in a role-specific menu, disable the main menu
            // (main menu only appears after logout)
            if (role == 1) {
                runAdminMenu();
            } else {
                runJudgeMenu(role);
            }
            role = 0; // Reset after logout so the main menu returns
        }
    }

    return 0;
}

// ---------- Menu printers ----------
void printMainMenu() {
    printf("\n====================================\n");
    printf("        JUDGING SYSTEM\n");
    printf("====================================\n");
    printf("\n[1] Admin\n[2] Judge 1\n[3] Judge 2\n[4] Judge 3\n[5] Exit\n");
}

void printJudgeMenu() {
    printf("\n====================================\n");
    printf("           JUDGE MENU\n");
    printf("====================================\n");
    printf("\n[1] View Rubrics\n[2] Score Contestants\n[3] View My Scoring Sheet\n[4] Change Username\n[5] Change Password\n[6] Logout\n");
}

void printAdminMenu() {
    printf("\n====================================\n");
    printf("           ADMIN MENU\n");
    printf("====================================\n");
    printf("\n[1] View Rubrics\n[2] View Overall Scoring Sheet\n[3] View Contestant Rankings\n[4] Display Judge Accounts\n[5] Logout\n");
}

// ---------- Rubrics ----------
void printRubricsText() {
    printf("\n====================================\n");
    printf("              RUBRICS\n");
    printf("====================================\n");
    int i;
    for (i = 0; i < NUM_CRITERIA; i++) {
        printf("\n%s       - %.0f%%\n", criterionNames[i], criterionWeights[i] * 100);
    }
    printf("\nTotal              - 100%%\n");
}

void viewRubrics() {
    printRubricsText();
}

// ---------- Login ----------
void login(int *role, int lockedRole) {
    char user[MAX_USERNAME + MAX_EMAIL + 4];
    char pass[MAX_PASSWORD + 4];
    int maxAttempts = 3;
    int attempts = 0;

    while (attempts < maxAttempts) {
        printf("\n--- Account Login ---\n");
        printf("Enter username or email: ");
        scanf("%s", user);
        printf("Enter password: ");
        scanf("%s", pass);

        // If locked to a specific role, only check that role's credentials
        if (lockedRole != 0) {
            if (lockedRole == 1) {
                // Admin only - must use admin credentials
                if ((strcmp(user, adminUsername) == 0 || strcmp(user, adminEmail) == 0) && strcmp(pass, adminPassword) == 0) {
                    printf("\nLogin successful! Welcome, %s.\n", adminUsername);
                    *role = 1;
                    return;
                }
            } else {
                // Judge only (locked to that specific judge)
                if ((strcmp(user, judgeUsernames[lockedRole - 2]) == 0 || strcmp(user, judgeEmails[lockedRole - 2]) == 0) && strcmp(pass, judgePasswords[lockedRole - 2]) == 0) {
                    printf("\nLogin successful! Welcome, %s.\n", judgeUsernames[lockedRole - 2]);
                    *role = lockedRole;
                    return;
                }
            }
            printf("\nInvalid username or password.\n");
            attempts++;
            if (attempts < maxAttempts) {
                printf("You have %d more attempt(s).\n", maxAttempts - attempts);
            }
            continue;
        }

        // Open login - check all accounts
        // Admin
        if ((strcmp(user, adminUsername) == 0 || strcmp(user, adminEmail) == 0) && strcmp(pass, adminPassword) == 0) {
            printf("\nLogin successful! Welcome, %s.\n", adminUsername);
            *role = 1;
            return;
        }

        // Judges
        int j;
        for (j = 0; j < NUM_JUDGES; j++) {
            if ((strcmp(user, judgeUsernames[j]) == 0 || strcmp(user, judgeEmails[j]) == 0) && strcmp(pass, judgePasswords[j]) == 0) {
                printf("\nLogin successful! Welcome, %s.\n", judgeUsernames[j]);
                *role = j + 2; // 2, 3, or 4
                return;
            }
        }

        printf("\nInvalid username/email or password.\n");
        attempts++;
        if (attempts < maxAttempts) {
            printf("You have %d more attempt(s).\n", maxAttempts - attempts);
        }
    }

    printf("\nToo many failed attempts. Exiting.\n");
    *role = 0;
}

// ---------- Judge menu ----------
void runJudgeMenu(int judgeNum) {
    int choice;
    while (1) {
        printJudgeMenu();
        printf("\nEnter your choice: ");
        scanf(" %d", &choice);

        switch (choice) {
            case 1: viewRubrics(); break;
            case 2:
                printf("\n--- Judge %d: Score Contestants ---\n", judgeNum - 1);
                scoreContestants(judgeNum);
                break;
            case 3:
                printf("\n--- Judge %d: My Scoring Sheet ---\n", judgeNum - 1);
                viewMyScoringSheet(judgeNum);
                break;
            case 4:
                printf("\n--- Change Username ---\n");
                changeUsername(judgeNum);
                break;
            case 5:
                printf("\n--- Change Password ---\n");
                changePassword(judgeNum);
                break;
            case 6:
                printf("\nLogging out...\n");
                return;
            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    }
}

void scoreContestants(int judgeNum) {
    int c, i;
    printf("\n====================================\n");
    printf("        SCORE CONTESTANTS\n");
    printf("====================================\n");

    for (c = 0; c < NUM_CONTESTANTS; c++) {
        printf("\nContestant %d:\n", c + 1);
        for (i = 0; i < NUM_CRITERIA; i++) {
            float score;
            printf("  Score for %s (1-100): ", criterionNames[i]);
            scanf("%f", &score);

            // Basic range validation
            if (score < 0) score = 0;
            if (score > 100) score = 100;

            judgeScores[judgeNum-1][i][c] = score;
        }
    }

    printf("\nScores saved successfully.\n");
}

void viewMyScoringSheet(int judgeNum) {
    int c, i;
    float final;

    printf("\n====================================\n");
    printf("       MY SCORING SHEET\n");
    printf("====================================\n");
    for (c = 0; c < NUM_CONTESTANTS; c++) {
        printf("\nContestant %d:\n", c + 1);
        for (i = 0; i < NUM_CRITERIA; i++) {
            printf("  %s        = %.0f\n", criterionNames[i], judgeScores[judgeNum-1][i][c]);
        }
        final = computeFinalScore(judgeScores[judgeNum-1][0][c],
                                   judgeScores[judgeNum-1][1][c],
                                   judgeScores[judgeNum-1][2][c],
                                   judgeScores[judgeNum-1][3][c]);
        printf("  Final Score        = %.2f\n", final);
    }
    printf("\n====================================\n");
}

void changeUsername(int roleType) {
    char newName[MAX_USERNAME];
    printf("\nCurrent Username: %s\n", roleType == 1 ? adminUsername : judgeUsernames[roleType - 2]);
    printf("Enter New Username: ");
    scanf("%s", newName);

    if (roleType == 1) {
        strcpy(adminUsername, newName);
    } else {
        strcpy(judgeUsernames[roleType - 2], newName);
    }

    printf("\nUsername successfully changed.\n");
}

void changePassword(int roleType) {
    char current[MAX_PASSWORD];
    char newPass[MAX_PASSWORD];
    printf("\nEnter Current Password: ");
    scanf("%s", current);
    printf("Enter New Password: ");
    scanf("%s", newPass);

    if (roleType == 1) {
        if (strcmp(current, adminPassword) == 0) {
            strcpy(adminPassword, newPass);
            printf("\nPassword successfully changed.\n");
        } else {
            printf("\nIncorrect password.\n");
        }
    } else {
        int j;
        for (j = 0; j < NUM_JUDGES; j++) {
            if (strcmp(current, judgePasswords[j]) == 0) {
                strcpy(judgePasswords[j], newPass);
                printf("\nPassword successfully changed.\n");
                return;
            }
        }
        printf("\nIncorrect password.\n");
    }
}

// ---------- Admin menu ----------
void runAdminMenu() {
    int choice;
    while (1) {
        printAdminMenu();
        printf("\nEnter your choice: ");
        scanf(" %d", &choice);

        switch (choice) {
            case 1: viewRubrics(); break;
            case 2:
                printf("\n--- Overall Scoring Sheet ---\n");
                viewOverallScoringSheet();
                break;
            case 3:
                printf("\n--- Contestant Rankings ---\n");
                viewContestantRankings();
                break;
            case 4: {
                printf("\n--- Account Verification ---\n");
                displayJudgeAccounts();
                break;
            }
            case 5:
                printf("\nLogging out...\n");
                return;
            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    }
}

void viewOverallScoringSheet() {
    int c;
    float sum, avg;

    printf("\n========================================================\n");
    printf("              OVERALL SCORING SHEET\n");
    printf("========================================================\n");
    printf("\nContestant     Judge 1     Judge 2     Judge 3     Average\n");
    for (c = 0; c < NUM_CONTESTANTS; c++) {
        float sum = 0;
        int j;
        for (j = 0; j < NUM_JUDGES; j++) {
            float f = computeFinalScore(getJudgeCriterionScore(j, c, 0),
                                        getJudgeCriterionScore(j, c, 1),
                                        getJudgeCriterionScore(j, c, 2),
                                        getJudgeCriterionScore(j, c, 3));
            sum += f;
        }
        avg = sum / NUM_JUDGES;
        printf("\n    %d             %.2f          %.2f          %.2f         %.2f", c + 1,
               computeFinalScore(getJudgeCriterionScore(0, c, 0),
                                  getJudgeCriterionScore(0, c, 1),
                                  getJudgeCriterionScore(0, c, 2),
                                  getJudgeCriterionScore(0, c, 3)),
               computeFinalScore(getJudgeCriterionScore(1, c, 0),
                                  getJudgeCriterionScore(1, c, 1),
                                  getJudgeCriterionScore(1, c, 2),
                                  getJudgeCriterionScore(1, c, 3)),
               computeFinalScore(getJudgeCriterionScore(2, c, 0),
                                  getJudgeCriterionScore(2, c, 1),
                                  getJudgeCriterionScore(2, c, 2),
                                  getJudgeCriterionScore(2, c, 3)),
               avg);
    }
    printf("\n\n========================================================\n");
}

void viewContestantRankings() {
    float averages[NUM_CONTESTANTS];
    int c;

    // Compute final average for each contestant
    for (c = 0; c < NUM_CONTESTANTS; c++) {
        float sum = 0;
        int j;
        for (j = 0; j < NUM_JUDGES; j++) {
            sum += computeFinalScore(getJudgeCriterionScore(j, c, 0),
                                      getJudgeCriterionScore(j, c, 1),
                                      getJudgeCriterionScore(j, c, 2),
                                      getJudgeCriterionScore(j, c, 3));
        }
        averages[c] = sum / NUM_JUDGES;
    }

    int ranks[NUM_CONTESTANTS];
    for (c = 0; c < NUM_CONTESTANTS; c++) ranks[c] = c + 1;

    // Simple ranking using if-else comparisons (bubble sort style)
    int a, b;
    for (a = 0; a < NUM_CONTESTANTS - 1; a++) {
        for (b = 0; b < NUM_CONTESTANTS - a - 1; b++) {
            if (averages[b] < averages[b + 1]) {
                float temp = averages[b];
                averages[b] = averages[b + 1];
                averages[b + 1] = temp;
                int t = ranks[b];
                ranks[b] = ranks[b + 1];
                ranks[b + 1] = t;
            }
        }
    }

    printf("\n====================================\n");
    printf("       CONTESTANT RANKINGS\n");
    printf("====================================\n");
    int r;
    for (r = 0; r < NUM_CONTESTANTS; r++) {
        printf("Rank %d - Contestant %d - %.2f\n", r + 1, ranks[r], averages[r]);
    }
    printf("\n====================================\n");
}

void displayJudgeAccounts() {
    int code;
    printf("\n====================================\n");
    printf("       ACCOUNT VERIFICATION\n");
    printf("====================================\n");
    printf("Enter Verification Code: ");
    scanf("%d", &code);

    if (code == 1234) {
        printf("\n====================================\n");
        printf("          JUDGE ACCOUNTS\n");
        printf("====================================\n");
        int j;
        for (j = 0; j < NUM_JUDGES; j++) {
            printf("\nJudge %d\nUsername: %s\nPassword: %s\n", j + 1, judgeUsernames[j], judgePasswords[j]);
        }
        printf("\n====================================\n");
    } else {
        printf("\nIncorrect verification code.\nAccess denied.\n");
    }
}

// ---------- Helpers ----------
char* getRoleName(int role) {
    if (role == 1) return "Admin";
    if (role == 2) return "Judge 1";
    if (role == 3) return "Judge 2";
    if (role == 4) return "Judge 3";
    return "Unknown";
}

float getJudgeCriterionScore(int judge, int c, int criterion) {
    if (criterion == 0) return judgeScores[judge][0][c];
    if (criterion == 1) return judgeScores[judge][1][c];
    if (criterion == 2) return judgeScores[judge][2][c];
    return judgeScores[judge][3][c];
}

float computeFinalScore(float perf, float creativity, float originality, float impact) {
    return (perf * 0.40) + (creativity * 0.30) + (originality * 0.20) + (impact * 0.10);
}
