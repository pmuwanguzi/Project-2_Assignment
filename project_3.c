
/*
 * Dynamic IoT Event Monitoring and Callback Engine
 *
 * Compile: gcc project_3.c -o project_3_Dynamic_IoT_Event_Monitoring_and_Callback_Engine
 * Run:     ./project_3_Dynamic_IoT_Event_Monitoring_and_Callback_Engine
 *
 * Custom callback: drift_detector()
 *   Purpose:   Detects a sudden jump between two consecutive readings of the
 *              same device. This can mean a faulty or loose sensor, even when
 *              both readings are still inside the normal range.
 *   Condition: |current reading - previous reading| > drift limit
 *              (Temperature 10 C, Pressure 30 kPa, Battery 0.5 V)
 *   Algorithm: 1. If the device has no previous reading, just save this one.
 *              2. Otherwise calculate the change from the previous reading.
 *              3. If the change is bigger than the limit, set the status to
 *                 ALERT and record a drift alert.
 *              4. Save the current reading as the previous reading.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
 
/* Device types */
#define TEMPERATURE 0
#define PRESSURE    1
#define BATTERY     2
 
/* Device status */
#define NORMAL 0
#define ALERT  1
 
/* Ranges used to simulate readings */
#define TEMP_MIN 10.0
#define TEMP_MAX 50.0
#define PRES_MIN 70
#define PRES_MAX 140
#define VOLT_MIN 2.9
#define VOLT_MAX 4.4
 
/* Normal operating limits */
#define TEMP_LOW  18.0
#define TEMP_HIGH 38.0
#define PRES_LOW  90
#define PRES_HIGH 120
#define VOLT_LOW  3.3
#define VOLT_HIGH 4.2
 
#define MAX_QUEUE  100
#define MAX_ALERTS 200
 
struct Device {
    int id;
    char name[40];
    int type;
    union {
        float temperature;
        int pressure;
        float voltage;
    } readings;
    int status;
    float previous;      /* previous reading (used by drift_detector) */
    int has_previous;    /* 1 if previous holds a value */
    int reading_count;
};
 
/* A generated reading waiting to be processed */
struct Reading {
    int device_id;
    float value;
};
 
/* One recorded alert */
struct Alert {
    int device_id;
    char name[40];
    int type;
    float value;
    char expected[40];
    char reason[80];
    char callback[30];
};
 
/* Global data */
struct Device *devices = NULL;   /* dynamic array of devices */
int device_count = 0;
 
struct Reading queue[MAX_QUEUE];
int queue_count = 0;
 
struct Alert alerts[MAX_ALERTS];
int alert_count = 0;
 
int total_readings = 0;
int normal_count = 0;
int abnormal_count = 0;
int alerts_per_type[3] = {0, 0, 0};
int drift_alerts = 0;
 
const char *type_names[3] = {"Temperature", "Pressure", "Battery"};
const char *units[3] = {"C", "kPa", "V"};
const char *callback_names[3] = {"temperature_monitor", "pressure_monitor", "battery_monitor"};
float drift_limits[3] = {10.0, 30.0, 0.5};
 
/* Input helpers */
void clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}
 
int read_int(void)
{
    int x, r;
    while ((r = scanf("%d", &x)) != 1) {
        if (r == EOF)
            return 0;           /* end of input -> behaves like Exit */
        clear_input();
        printf("Please enter a number: ");
    }
    clear_input();
    return x;
}
 
/* Union helpers: always use the member matching the type */
float get_reading(struct Device *d)
{
    switch (d->type) {
    case TEMPERATURE: return d->readings.temperature;
    case PRESSURE:    return d->readings.pressure;
    case BATTERY:     return d->readings.voltage;
    }
    return 0;
}
 
void set_reading(struct Device *d, float value)
{
    switch (d->type) {
    case TEMPERATURE: d->readings.temperature = value; break;
    case PRESSURE:    d->readings.pressure = (int)value; break;
    case BATTERY:     d->readings.voltage = value; break;
    }
}
 
float random_float(float min, float max)
{
    return min + ((float)rand() / RAND_MAX) * (max - min);
}
 
/* Find a device by ID using pointer traversal */
struct Device *find_device(int id)
{
    struct Device *p;
    for (p = devices; p < devices + device_count; p++) {
        if (p->id == id)
            return p;
    }
    return NULL;
}
 
/* Alerts */
void add_alert(struct Device *d, float value, char *expected, char *reason, char *callback)
{
    d->status = ALERT;
    alerts_per_type[d->type]++;
 
    if (alert_count >= MAX_ALERTS) {
        printf("Alert log is full.\n");
        return;
    }
    alerts[alert_count].device_id = d->id;
    strcpy(alerts[alert_count].name, d->name);
    alerts[alert_count].type = d->type;
    alerts[alert_count].value = value;
    strcpy(alerts[alert_count].expected, expected);
    strcpy(alerts[alert_count].reason, reason);
    strcpy(alerts[alert_count].callback, callback);
    alert_count++;
}
 
/* Callback functions */
void temperature_monitor(struct Device *d)
{
    float t = d->readings.temperature;
    char expected[40];
 
    sprintf(expected, "%.1f - %.1f C", TEMP_LOW, TEMP_HIGH);
    if (t > TEMP_HIGH)
        add_alert(d, t, expected, "Temperature too high (overheating)", "temperature_monitor");
    else if (t < TEMP_LOW)
        add_alert(d, t, expected, "Temperature too low", "temperature_monitor");
}
 
void pressure_monitor(struct Device *d)
{
    int p = d->readings.pressure;
    char expected[40];
 
    sprintf(expected, "%d - %d kPa", PRES_LOW, PRES_HIGH);
    if (p > PRES_HIGH)
        add_alert(d, p, expected, "Pressure too high", "pressure_monitor");
    else if (p < PRES_LOW)
        add_alert(d, p, expected, "Pressure too low (possible leak)", "pressure_monitor");
}
 
void battery_monitor(struct Device *d)
{
    float v = d->readings.voltage;
    char expected[40];
 
    sprintf(expected, "%.2f - %.2f V", VOLT_LOW, VOLT_HIGH);
    if (v < VOLT_LOW)
        add_alert(d, v, expected, "Battery voltage critically low", "battery_monitor");
    else if (v > VOLT_HIGH)
        add_alert(d, v, expected, "Overvoltage (charging fault)", "battery_monitor");
}
 
/* Custom callback: see explanation at the top of the file */
void drift_detector(struct Device *d)
{
    float current = get_reading(d);
    float change;
    char expected[40], reason[80];
 
    if (d->has_previous) {
        change = fabs(current - d->previous);
        if (change > drift_limits[d->type]) {
            sprintf(expected, "change within %.1f %s", drift_limits[d->type], units[d->type]);
            sprintf(reason, "Sudden change of %.2f %s (previous %.2f)",
                    change, units[d->type], d->previous);
            add_alert(d, current, expected, reason, "drift_detector");
            drift_alerts++;
        }
    }
    d->previous = current;
    d->has_previous = 1;
}
 
/* Array of function pointers: index = device type */
void (*callbacks[3])(struct Device *) = {temperature_monitor, pressure_monitor, battery_monitor};
 
void process_device(struct Device *device, void (*callback)(struct Device *))
{
    callback(device);
}
 
/* Menu operations*/
int add_device(int id, char *name, int type)
{
    struct Device *temp;
 
    if (find_device(id) != NULL) {
        printf("Error: device ID %d already exists.\n", id);
        return 0;
    }
 
    /* grow the array by one device */
    temp = realloc(devices, (device_count + 1) * sizeof(struct Device));
    if (temp == NULL) {
        printf("Error: memory allocation failed.\n");
        return 0;
    }
    devices = temp;
 
    devices[device_count].id = id;
    strncpy(devices[device_count].name, name, 39);
    devices[device_count].name[39] = '\0';
    devices[device_count].type = type;
    devices[device_count].status = NORMAL;
    devices[device_count].has_previous = 0;
    devices[device_count].previous = 0;
    devices[device_count].reading_count = 0;
    device_count++;
 
    printf("Device %d (%s) registered. Total devices: %d\n", id, name, device_count);
    return 1;
}
 
void register_device(void)
{
    int id, type;
    char name[40];
 
    printf("Enter device ID: ");
    id = read_int();
    if (id <= 0) {
        printf("Error: ID must be a positive number.\n");
        return;
    }
    if (find_device(id) != NULL) {
        printf("Error: device ID %d already exists.\n", id);
        return;
    }
 
    printf("Enter device name: ");
    if (fgets(name, sizeof(name), stdin) == NULL)
        return;
    name[strcspn(name, "\n")] = '\0';
 
    printf("Device type (1 = Temperature, 2 = Pressure, 3 = Battery): ");
    type = read_int();
    if (type < 1 || type > 3) {
        printf("Error: invalid device type.\n");
        return;
    }
 
    add_device(id, name, type - 1);
}
 
void load_demo_devices(void)
{
    add_device(101, "Boiler Room Temp", TEMPERATURE);
    add_device(102, "Server Rack Temp", TEMPERATURE);
    add_device(201, "Water Main Pressure", PRESSURE);
    add_device(202, "Air Compressor", PRESSURE);
    add_device(301, "Gateway Battery", BATTERY);
    add_device(302, "Field Node Battery", BATTERY);
}
 
void display_devices(void)
{
    struct Device *p;
 
    if (device_count == 0) {
        printf("No devices registered.\n");
        return;
    }
    printf("\n%-6s %-20s %-12s %-10s %-8s %s\n",
           "ID", "Name", "Type", "Reading", "Status", "Readings");
    printf("----------------------------------------------------------------------\n");
    for (p = devices; p < devices + device_count; p++) {
        printf("%-6d %-20s %-12s ", p->id, p->name, type_names[p->type]);
        if (p->reading_count == 0)
            printf("%-10s ", "--");
        else
            printf("%-6.2f %-3s ", get_reading(p), units[p->type]);
        printf("%-8s %d\n", p->status == NORMAL ? "NORMAL" : "ALERT", p->reading_count);
    }
}
 
void remove_device(void)
{
    int id, i, index = -1;
    struct Device *temp;
 
    printf("Enter device ID to remove: ");
    id = read_int();
 
    for (i = 0; i < device_count; i++) {
        if (devices[i].id == id) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        printf("Device %d not found.\n", id);
        return;
    }
 
    /* shift the devices after it one place to the left */
    for (i = index; i < device_count - 1; i++)
        devices[i] = devices[i + 1];
    device_count--;
 
    /* shrink the array */
    if (device_count == 0) {
        free(devices);
        devices = NULL;
    } else {
        temp = realloc(devices, device_count * sizeof(struct Device));
        if (temp != NULL)
            devices = temp;     /* if it fails, the old memory is still valid */
    }
 
    /* remove any waiting readings for this device */
    int j = 0;
    for (i = 0; i < queue_count; i++) {
        if (queue[i].device_id != id)
            queue[j++] = queue[i];
    }
    queue_count = j;
 
    printf("Device %d removed. Total devices: %d\n", id, device_count);
}
 
void generate_readings(void)
{
    int n, i, index;
    struct Device *d;
    float value;
 
    if (device_count == 0) {
        printf("Register devices first.\n");
        return;
    }
    printf("How many readings to generate (at least 10)? ");
    n = read_int();
    if (n < 1)
        return;
    if (queue_count + n > MAX_QUEUE)
        n = MAX_QUEUE - queue_count;
 
    printf("\n%-4s %-6s %-20s %-12s %s\n", "No", "ID", "Name", "Type", "Value");
    for (i = 0; i < n; i++) {
        /* every device gets one reading first, then random devices */
        if (i < device_count)
            index = i;
        else
            index = rand() % device_count;
        d = &devices[index];
 
        if (d->type == TEMPERATURE)
            value = random_float(TEMP_MIN, TEMP_MAX);
        else if (d->type == PRESSURE)
            value = PRES_MIN + rand() % (PRES_MAX - PRES_MIN + 1);
        else
            value = random_float(VOLT_MIN, VOLT_MAX);
 
        queue[queue_count].device_id = d->id;
        queue[queue_count].value = value;
        queue_count++;
 
        printf("%-4d %-6d %-20s %-12s %.2f %s\n",
               i + 1, d->id, d->name, type_names[d->type], value, units[d->type]);
    }
    printf("%d readings waiting. Choose option 5 to process them.\n", queue_count);
}
 
void process_readings(void)
{
    int i, j, before;
    struct Device *d;
 
    if (queue_count == 0) {
        printf("No readings to process. Generate readings first.\n");
        return;
    }
 
    for (i = 0; i < queue_count; i++) {
        d = find_device(queue[i].device_id);
        if (d == NULL)
            continue;
 
        set_reading(d, queue[i].value);
        d->status = NORMAL;
        d->reading_count++;
        before = alert_count;
 
        /* choose the callback using the function pointer array */
        process_device(d, callbacks[d->type]);
        process_device(d, drift_detector);
 
        total_readings++;
        if (d->status == NORMAL)
            normal_count++;
        else
            abnormal_count++;
 
        printf("\nReading %d: ID %d | %s | %s | %.2f %s | %s\n",
               i + 1, d->id, d->name, type_names[d->type],
               get_reading(d), units[d->type],
               d->status == NORMAL ? "NORMAL" : "ALERT");
        if (alert_count == before)
            printf("   -> Within limits [%s, drift_detector]\n", callback_names[d->type]);
        for (j = before; j < alert_count; j++)
            printf("   -> %s [%s]\n", alerts[j].reason, alerts[j].callback);
    }
    queue_count = 0;
}
 
void display_alerts(void)
{
    int i;
 
    if (alert_count == 0) {
        printf("No alerts.\n");
        return;
    }
    for (i = 0; i < alert_count; i++) {
        printf("\nAlert %d\n", i + 1);
        printf("  Device   : %d (%s)\n", alerts[i].device_id, alerts[i].name);
        printf("  Value    : %.2f %s\n", alerts[i].value, units[alerts[i].type]);
        printf("  Expected : %s\n", alerts[i].expected);
        printf("  Reason   : %s\n", alerts[i].reason);
        printf("  Callback : %s\n", alerts[i].callback);
    }
}
 
void show_summary(void)
{
    int i;
 
    printf("\n========== MONITORING SUMMARY ==========\n");
    printf("Registered devices : %d\n", device_count);
    printf("Readings processed : %d\n", total_readings);
    printf("Normal readings    : %d\n", normal_count);
    printf("Abnormal readings  : %d\n", abnormal_count);
    printf("Alerts per device type:\n");
    for (i = 0; i < 3; i++)
        printf("  %-12s : %d\n", type_names[i], alerts_per_type[i]);
    printf("Custom callback (drift_detector):\n");
    printf("  Drift alerts     : %d\n", drift_alerts);
    printf("  Drift limits     : %.1f C, %.0f kPa, %.1f V\n",
           drift_limits[0], drift_limits[1], drift_limits[2]);
    printf("========================================\n");
}
 
int main(void)
{
    int choice;
 
    srand(time(NULL));
 
    do {
        printf("\n===== IoT Monitoring Menu =====\n");
        printf("1. Register device\n");
        printf("2. Display devices\n");
        printf("3. Remove device\n");
        printf("4. Generate readings\n");
        printf("5. Process readings\n");
        printf("6. Display alerts\n");
        printf("7. Monitoring summary\n");
        printf("8. Load demo devices\n");
        printf("0. Exit\n");
        printf("Choice: ");
        choice = read_int();
 
        switch (choice) {
        case 1: register_device(); break;
        case 2: display_devices(); break;
        case 3: remove_device(); break;
        case 4: generate_readings(); break;
        case 5: process_readings(); break;
        case 6: display_alerts(); break;
        case 7: show_summary(); break;
        case 8: load_demo_devices(); break;
        case 0: break;
        default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
 
    free(devices);
    devices = NULL;
    printf("Memory freed. Goodbye!\n");
    return 0;
}
 

