/*
 * Dynamic Academic Performance and Risk Analysis Engine
 * Author : Paul Muwanguzi
 * Compile: gcc project_1.c -o Project1_Dynamic_Academic_Performance_and_Risk_Analysis_Engine
 * Run    : ./Project1_Dynamic_Academic_Performance_and_Risk_Analysis_Engine
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STUDENT_ID  "p.muwanguzi@alustudent.com"   /* my student ID */
#define NAME_SIZE   50
#define COURSE_SIZE 30
#define PASS_MARK   50.0       /* minimum grade to pass */

/* Grade at or above this has no risk. Default 70, user can change it
   when running the custom analysis (option 8). */
float safe_mark = 70.0;

typedef struct {
    int   id;
    char  name[NAME_SIZE];
    char  course[COURSE_SIZE];
    float grade;
} Student;

/* Global dynamic array of students and the number of records in it */
Student *students = NULL;
int count = 0;


/*Input helper functions*/

/* Reads a line safely into buffer (never writes past size). */
void read_string(const char *prompt, char *buffer, int size)
{
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL) {
        printf("\nEnd of input. Exiting.\n");
        free(students);
        exit(0);
    }
    if (strchr(buffer, '\n') == NULL) {
        int c;                      /* input too long: discard the rest */
        while ((c = getchar()) != '\n' && c != EOF)
            ;
        printf("(Input too long - only the first %d characters were kept.)\n",
               size - 1);
    } else {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

/* Keeps asking until the user types a valid whole number. */
int read_int(const char *prompt)
{
    char line[50], extra;
    int value;

    while (1) {
        read_string(prompt, line, sizeof(line));
        if (sscanf(line, "%d %c", &value, &extra) == 1)
            return value;
        printf("Invalid input. Please enter a whole number.\n");
    }
}

/* Keeps asking until the user types a valid number. */
float read_float(const char *prompt)
{
    char line[50], extra;
    float value;

    while (1) {
        read_string(prompt, line, sizeof(line));
        if (sscanf(line, "%f %c", &value, &extra) == 1)
            return value;
        printf("Invalid input. Please enter a number.\n");
    }
}

/* Returns the index of the student with this ID, or -1 if not found. */
int find_index(int id)
{
    int i;
    for (i = 0; i < count; i++)
        if (students[i].id == id)
            return i;
    return -1;
}

void print_header_row(void)
{
    printf("%-6s %-22s %-20s %6s\n", "ID", "Name", "Course", "Grade");
    printf("-------------------------------------------------------\n");
}

void print_student(const Student *s)
{
    printf("%-6d %-22s %-20s %6.1f\n", s->id, s->name, s->course, s->grade);
}

/*Initial dataset */

int load_initial_data(void)
{
    Student data[] = {
        {1001, "Aline Uwase",          "Programming in C",  88.0},
        {1002, "Jean Bosco Habimana",  "Programming in C",  47.5},
        {1003, "Grace Mukamana",       "Data Structures",   73.0},
        {1004, "Eric Niyonzima",       "Database Systems",  61.0},
        {1005, "Diane Ingabire",       "Computer Networks", 92.5},
        {1006, "Patrick Mugisha",      "Data Structures",   38.0},
        {1007, "Claudine Uwimana",     "Database Systems",  79.0},
        {1008, "Olivier Nshimiyimana", "Computer Networks", 55.0},
        {1009, "Sandrine Iradukunda",  "Programming in C",  66.0},
        {1010, "Kevin Byiringiro",     "Database Systems",  29.5},
        {1011, "Esther Umutoni",       "Computer Networks", 70.0},
        {1012, "David Kwizera",        "Data Structures",   84.0}
    };
    int n = sizeof(data) / sizeof(data[0]);

    students = (Student *)calloc(n, sizeof(Student));
    if (students == NULL)
        return 0;

    memcpy(students, data, sizeof(data));
    count = n;
    return 1;
}

/*Menu operations*/

void add_student(void)
{
    Student s;
    Student *temp;

    printf("\n--- Add Student ---\n");

    /* ID must be positive and not already used */
    while (1) {
        s.id = read_int("Enter student ID: ");
        if (s.id <= 0)
            printf("ID must be a positive number.\n");
        else if (find_index(s.id) != -1)
            printf("Duplicate ID. Student %d already exists.\n", s.id);
        else
            break;
    }

    while (1) {
        read_string("Enter full name: ", s.name, NAME_SIZE);
        if (strlen(s.name) > 0)
            break;
        printf("Name cannot be empty.\n");
    }

    while (1) {
        read_string("Enter course: ", s.course, COURSE_SIZE);
        if (strlen(s.course) > 0)
            break;
        printf("Course cannot be empty.\n");
    }

    /* grade must be between 0 and 100 */
    while (1) {
        s.grade = read_float("Enter grade (0-100): ");
        if (s.grade >= 0 && s.grade <= 100)
            break;
        printf("Invalid grade. Must be between 0 and 100.\n");
    }

    /* grow the array by one record */
    temp = (Student *)realloc(students, (count + 1) * sizeof(Student));
    if (temp == NULL) {
        printf("Memory allocation failed. Student not added.\n");
        return;
    }
    students = temp;
    students[count] = s;
    count++;

    printf("Student added successfully. Total records: %d\n", count);
}

/* Uses pointer traversal instead of array indexing */
void display_students(void)
{
    Student *p;

    printf("\n--- All Students (%d) ---\n", count);
    if (count == 0) {
        printf("No records found.\n");
        return;
    }
    print_header_row();
    for (p = students; p < students + count; p++)
        print_student(p);
}

/* Comparison functions used through a function pointer */
int compare_grade(const Student *a, const Student *b)
{
    return a->grade < b->grade;          /* highest grade first */
}

int compare_name(const Student *a, const Student *b)
{
    return strcmp(a->name, b->name) > 0; /* alphabetical order */
}

int compare_id(const Student *a, const Student *b)
{
    return a->id > b->id;                /* smallest ID first */
}

/* Bubble sort that uses whichever compare function is passed in */
void bubble_sort(int (*compare)(const Student *, const Student *))
{
    int i, j;
    Student temp;

    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - 1 - i; j++) {
            if (compare(&students[j], &students[j + 1])) {
                temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
}

void sort_students(void)
{
    int option;

    printf("\n--- Sort Students ---\n");
    printf("1. By grade\n2. By name\n3. By ID\n");
    option = read_int("Choose option: ");

    if (option == 1)
        bubble_sort(compare_grade);
    else if (option == 2)
        bubble_sort(compare_name);
    else if (option == 3)
        bubble_sort(compare_id);
    else {
        printf("Invalid option.\n");
        return;
    }
    display_students();
}

/* Average grade calculated with pointer traversal */
float get_average(void)
{
    Student *p;
    float total = 0;

    if (count == 0)
        return 0;
    for (p = students; p < students + count; p++)
        total += p->grade;
    return total / count;
}

void calculate_average(void)
{
    printf("\n--- Average Grade ---\n");
    if (count == 0) {
        printf("No records found.\n");
        return;
    }
    printf("Average grade of %d students: %.2f\n", count, get_average());
}

void find_top_student(void)
{
    Student *p, *top;

    printf("\n--- Top Student ---\n");
    if (count == 0) {
        printf("No records found.\n");
        return;
    }
    top = students;
    for (p = students + 1; p < students + count; p++)
        if (p->grade > top->grade)
            top = p;

    print_header_row();
    print_student(top);
}

void search_student(void)
{
    int option, id, i, found = 0;
    char name[NAME_SIZE];

    printf("\n--- Search Student ---\n");
    printf("1. By ID\n2. By name\n");
    option = read_int("Choose option: ");

    if (option == 1) {
        id = read_int("Enter student ID: ");
        i = find_index(id);
        if (i != -1) {
            print_header_row();
            print_student(&students[i]);
            found = 1;
        }
    } else if (option == 2) {
        read_string("Enter full name: ", name, NAME_SIZE);
        for (i = 0; i < count; i++) {
            if (strcmp(students[i].name, name) == 0) {
                if (!found)
                    print_header_row();
                print_student(&students[i]);
                found = 1;
            }
        }
    } else {
        printf("Invalid option.\n");
        return;
    }

    if (!found)
        printf("Student not found.\n");
}

void remove_student(void)
{
    int id, index, i;
    Student *temp;

    printf("\n--- Remove Student ---\n");
    id = read_int("Enter ID of student to remove: ");
    index = find_index(id);

    if (index == -1) {
        printf("Student with ID %d not found.\n", id);
        return;
    }

    printf("Removing: %s\n", students[index].name);

    /* shift the remaining records one position to the left */
    for (i = index; i < count - 1; i++)
        students[i] = students[i + 1];
    count--;

    /* shrink the array */
    if (count == 0) {
        free(students);
        students = NULL;
    } else {
        temp = (Student *)realloc(students, count * sizeof(Student));
        if (temp != NULL)
            students = temp;
    }
    printf("Student removed successfully. Total records: %d\n", count);
}

/*            Custom analysis: Academic Risk Index (ARI)             */
/*                                                                   */
/*  ARI = (safe_mark - grade) / safe_mark x 100   if grade < safe   */
/*  ARI = 0                                       if grade >= safe  */
/*                                                                   */
/*  Levels: High   - ARI >= 30, or grade below the pass mark (50)    */
/*          Medium - ARI 15 to 29.9                                  */
/*          Low    - ARI below 15                                    */
/*                                                                   */
/*  The analysis also groups students by course (using strcmp) to    */
/*  show which courses have the most students at risk.               */


float risk_score(float grade)
{
    if (grade >= safe_mark)
        return 0;
    return (safe_mark - grade) / safe_mark * 100;
}

/* A failing student is always High risk, whatever the score */
const char *risk_level(float score, float grade)
{
    if (score >= 30 || grade < PASS_MARK)
        return "High";
    else if (score >= 15)
        return "Medium";
    else
        return "Low";
}

/* Counts students in each risk level and returns the average risk */
float count_risk_levels(int *high, int *medium, int *low)
{
    int i;
    float score, total = 0;
    const char *level;

    *high = *medium = *low = 0;
    for (i = 0; i < count; i++) {
        score = risk_score(students[i].grade);
        level = risk_level(score, students[i].grade);
        total += score;
        if (strcmp(level, "High") == 0)
            (*high)++;
        else if (strcmp(level, "Medium") == 0)
            (*medium)++;
        else
            (*low)++;
    }
    return (count > 0) ? total / count : 0;
}

/* Shows number of students, average grade and students at risk
   (High or Medium) for each course */
void course_risk_breakdown(void)
{
    int i, j, seen, students_in_course, at_risk;
    float total;
    const char *level;

    printf("\nRisk by course:\n");
    printf("%-20s %8s %9s %8s\n", "Course", "Students", "Avg Grade", "At Risk");
    printf("--------------------------------------------------\n");

    for (i = 0; i < count; i++) {
        /* skip this course if it was already printed earlier */
        seen = 0;
        for (j = 0; j < i; j++)
            if (strcmp(students[j].course, students[i].course) == 0)
                seen = 1;
        if (seen)
            continue;

        students_in_course = 0;
        at_risk = 0;
        total = 0;
        for (j = i; j < count; j++) {
            if (strcmp(students[j].course, students[i].course) == 0) {
                students_in_course++;
                total += students[j].grade;
                level = risk_level(risk_score(students[j].grade), students[j].grade);
                if (strcmp(level, "Low") != 0)
                    at_risk++;
            }
        }
        printf("%-20s %8d %9.1f %8d\n", students[i].course,
               students_in_course, total / students_in_course, at_risk);
    }
}

void custom_analysis(void)
{
    int i, high, medium, low;
    float score, avg_risk, new_mark;

    printf("\n--- Academic Risk Index (ARI) ---\n");
    if (count == 0) {
        printf("No records found.\n");
        return;
    }

    /* let the user choose the safe mark */
    printf("Current safe mark: %.0f\n", safe_mark);
    while (1) {
        new_mark = read_float("Enter safe mark (50-100), or 0 to keep current: ");
        if (new_mark == 0)
            break;
        if (new_mark >= PASS_MARK && new_mark <= 100) {
            safe_mark = new_mark;
            break;
        }
        printf("Safe mark must be between 50 and 100.\n");
    }

    printf("\nFormula: ARI = (%.0f - grade) / %.0f x 100, and 0 if grade >= %.0f\n",
           safe_mark, safe_mark, safe_mark);
    printf("Levels : High >= 30 (or grade below %.0f), Medium 15-29.9, Low < 15\n\n",
           PASS_MARK);

    printf("%-6s %-22s %6s %6s  %-6s\n", "ID", "Name", "Grade", "ARI", "Level");
    printf("-----------------------------------------------------\n");
    for (i = 0; i < count; i++) {
        score = risk_score(students[i].grade);
        printf("%-6d %-22s %6.1f %6.1f  %-6s\n", students[i].id, students[i].name,
               students[i].grade, score, risk_level(score, students[i].grade));
    }

    avg_risk = count_risk_levels(&high, &medium, &low);
    printf("\nHigh risk: %d   Medium risk: %d   Low risk: %d\n", high, medium, low);
    printf("Average class risk: %.1f\n", avg_risk);

    course_risk_breakdown();

    printf("\nInterpretation: High risk students need urgent support (tutoring),\n");
    printf("Medium risk students should be monitored. Courses with many at-risk\n");
    printf("students may need a review of teaching or extra revision sessions.\n");
}

void summary_report(void)
{
    int i, passed = 0, high, medium, low;
    float highest, lowest, avg_risk;

    printf("\n========== SUMMARY REPORT ==========\n");
    printf("Total students      : %d\n", count);
    if (count == 0)
        return;

    highest = lowest = students[0].grade;
    for (i = 0; i < count; i++) {
        if (students[i].grade > highest) highest = students[i].grade;
        if (students[i].grade < lowest)  lowest  = students[i].grade;
        if (students[i].grade >= PASS_MARK) passed++;
    }
    avg_risk = count_risk_levels(&high, &medium, &low);

    printf("Average grade       : %.2f\n", get_average());
    printf("Highest grade       : %.1f\n", highest);
    printf("Lowest grade        : %.1f\n", lowest);
    printf("Pass mark           : %.0f\n", PASS_MARK);
    printf("Students passed     : %d\n", passed);
    printf("Students below pass : %d\n", count - passed);
    printf("--- Risk Analysis (safe mark %.0f) ---\n", safe_mark);
    printf("High / Medium / Low : %d / %d / %d\n", high, medium, low);
    printf("Average class risk  : %.1f\n", avg_risk);
    printf("====================================\n");
}

/* main / program entry */

int main(void)
{
    /* array of function pointers: menu choice n calls operations[n-1] */
    void (*operations[])(void) = {
        add_student, display_students, sort_students, calculate_average,
        find_top_student, search_student, remove_student, custom_analysis,
        summary_report
    };
    int choice;

    if (!load_initial_data()) {
        printf("Memory allocation failed. Program cannot start.\n");
        return 1;
    }

    printf("========================================================\n");
    printf("Student ID: %s\n", STUDENT_ID);
    printf("Project: Dynamic Academic Performance and Risk Analysis Engine\n");
    printf("Custom Analysis: Academic Risk Index\n");
    printf("========================================================\n");

    while (1) {
        printf("\n----------- MENU -----------\n");
        printf("1. Add student\n");
        printf("2. Display students\n");
        printf("3. Sort students\n");
        printf("4. Calculate average grade\n");
        printf("5. Find top student\n");
        printf("6. Search student\n");
        printf("7. Remove student\n");
        printf("8. Custom analysis (ARI)\n");
        printf("9. Summary report\n");
        printf("10. Exit\n");
        choice = read_int("Enter choice: ");

        if (choice == 10)
            break;
        if (choice >= 1 && choice <= 9)
            operations[choice - 1]();
        else
            printf("Invalid choice. Try again.\n");
    }

    free(students);        /* release all dynamic memory */
    students = NULL;
    printf("Memory freed. Goodbye!\n");
    return 0;
}
