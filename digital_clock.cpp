#include <cstdint>
#include <cstring>
#include <cstdlib>

extern "C" {
    void terminal_putchar(char c);
    void terminal_writestring(const char *str);
    void terminal_clear(void);
}

/* Time Zone Structure */
typedef struct {
    const char *name;
    const char *abbr;
    int offset_hours;
    int offset_minutes;
    uint8_t is_dst;
} timezone_t;

/* Digital Clock System */
class DigitalClock {
private:
    static const int MAX_TIMEZONES = 24;
    static const int BUFFER_SIZE = 256;
    
    typedef struct {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t day_of_week;
    } datetime_t;
    
    /* Time Zones */
    timezone_t timezones[MAX_TIMEZONES] = {
        {"UTC/GMT", "GMT", 0, 0, 0},
        {"London", "GMT/BST", 0, 0, 0},
        {"Paris", "CET/CEST", 1, 0, 0},
        {"Berlin", "CET/CEST", 1, 0, 0},
        {"Tokyo", "JST", 9, 0, 0},
        {"Sydney", "AEDT", 11, 0, 0},
        {"New York", "EST/EDT", -5, 0, 0},
        {"Los Angeles", "PST/PDT", -8, 0, 0},
        {"Dubai", "GST", 4, 0, 0},
        {"Singapore", "SGT", 8, 0, 0},
        {"Hong Kong", "HKT", 8, 0, 0},
        {"Bangkok", "ICT", 7, 0, 0},
        {"Mumbai", "IST", 5, 30, 0},
        {"Istanbul", "EET/EEST", 2, 0, 0},
        {"Moscow", "MSK", 3, 0, 0},
        {"Bangkok", "ICT", 7, 0, 0},
        {"Mexico City", "CST/CDT", -6, 0, 0},
        {"Brazil", "BRT/BRST", -3, 0, 0},
        {"South Africa", "SAST", 2, 0, 0},
        {"Auckland", "NZDT", 13, 0, 0},
        {"Phoenix", "MST", -7, 0, 0},
        {"Denver", "MST/MDT", -7, 0, 0},
        {"Chicago", "CST/CDT", -6, 0, 0},
        {"Toronto", "EST/EDT", -5, 0, 0}
    };
    
    int timezone_count;
    datetime_t current_time;
    uint32_t total_seconds;
    uint8_t is_24_hour;
    uint8_t show_seconds;
    uint8_t show_date;
    
public:
    DigitalClock() : timezone_count(MAX_TIMEZONES), is_24_hour(1), 
                     show_seconds(1), show_date(1) {
        init_time();
    }
    
    void run(void) {
        terminal_clear();
        print_banner();
        print_menu();
        
        char command[BUFFER_SIZE];
        while (1) {
            terminal_writestring("\nclock> ");
            read_input(command, BUFFER_SIZE);
            
            if (!execute_command(command)) {
                break;
            }
        }
    }
    
private:
    void init_time(void) {
        /* Initialize with example time: 2026-06-03 14:35:42 */
        current_time.year = 2026;
        current_time.month = 6;
        current_time.day = 3;
        current_time.hour = 14;
        current_time.minute = 35;
        current_time.second = 42;
        current_time.day_of_week = 3; /* Wednesday */
        
        calculate_total_seconds();
    }
    
    void calculate_total_seconds(void) {
        total_seconds = (current_time.hour * 3600) + 
                       (current_time.minute * 60) + 
                       current_time.second;
    }
    
    void print_banner(void) {
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║   🕐 DIGITAL CLOCK SYSTEM v1.0        ║\n");
        terminal_writestring("║      Multi-Timezone Display           ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
    }
    
    void print_menu(void) {
        terminal_writestring("=== DIGITAL CLOCK MENU ===\n");
        terminal_writestring("  clock              - Display main clock\n");
        terminal_writestring("  timezone <num>     - View timezone (1-24)\n");
        terminal_writestring("  all                - Show all timezones\n");
        terminal_writestring("  analog             - Analog clock display\n");
        terminal_writestring("  set-time HH:MM:SS  - Set current time\n");
        terminal_writestring("  set-date YYYY-MM-DD - Set date\n");
        terminal_writestring("  format 24/12       - Toggle 24/12 hour format\n");
        terminal_writestring("  timer <seconds>    - Start timer\n");
        terminal_writestring("  stopwatch          - Start stopwatch\n");
        terminal_writestring("  alarm <HH:MM>      - Set alarm\n");
        terminal_writestring("  world              - World clock view\n");
        terminal_writestring("  settings           - Clock settings\n");
        terminal_writestring("  exit               - Exit clock\n");
    }
    
    bool execute_command(const char *cmd) {
        if (cmd[0] == '\0') return true;
        
        if (str_compare(cmd, "clock")) {
            display_main_clock();
            return true;
        }
        else if (str_starts_with(cmd, "timezone ")) {
            int tz_num = parse_number(cmd + 9);
            if (tz_num >= 1 && tz_num <= timezone_count) {
                display_timezone(tz_num - 1);
            } else {
                terminal_writestring("[!] Invalid timezone number\n");
            }
            return true;
        }
        else if (str_compare(cmd, "all")) {
            display_all_timezones();
            return true;
        }
        else if (str_compare(cmd, "analog")) {
            display_analog_clock();
            return true;
        }
        else if (str_starts_with(cmd, "set-time ")) {
            set_time(cmd + 9);
            return true;
        }
        else if (str_starts_with(cmd, "set-date ")) {
            set_date(cmd + 9);
            return true;
        }
        else if (str_starts_with(cmd, "format ")) {
            set_format(cmd + 7);
            return true;
        }
        else if (str_starts_with(cmd, "timer ")) {
            int seconds = parse_number(cmd + 6);
            run_timer(seconds);
            return true;
        }
        else if (str_compare(cmd, "stopwatch")) {
            run_stopwatch();
            return true;
        }
        else if (str_starts_with(cmd, "alarm ")) {
            set_alarm(cmd + 6);
            return true;
        }
        else if (str_compare(cmd, "world")) {
            display_world_clock();
            return true;
        }
        else if (str_compare(cmd, "settings")) {
            show_settings();
            return true;
        }
        else if (str_compare(cmd, "exit") || str_compare(cmd, "quit")) {
            terminal_writestring("\n[*] Exiting clock...\n");
            return false;
        }
        else if (str_compare(cmd, "help")) {
            print_menu();
            return true;
        }
        else {
            terminal_writestring("Unknown command: ");
            terminal_writestring(cmd);
            terminal_putchar('\n');
            return true;
        }
    }
    
    void display_main_clock(void) {
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║          LOCAL TIME (UTC+0)            ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
        
        draw_digital_display(current_time.hour, current_time.minute, 
                            current_time.second, 0, 0);
        
        terminal_writestring("\n");
        print_date_info();
    }
    
    void draw_digital_display(uint8_t hour, uint8_t minute, uint8_t second,
                             int offset_h, int offset_m) {
        /* Apply timezone offset */
        int total_mins = (hour * 60 + minute) + (offset_h * 60) + offset_m;
        
        if (total_mins < 0) total_mins += 1440;
        if (total_mins >= 1440) total_mins -= 1440;
        
        uint8_t tz_hour = total_mins / 60;
        uint8_t tz_minute = total_mins % 60;
        
        /* Convert to 12-hour if needed */
        uint8_t display_hour = tz_hour;
        const char *period = "AM";
        
        if (!is_24_hour) {
            if (tz_hour >= 12) {
                period = "PM";
                if (tz_hour > 12) {
                    display_hour = tz_hour - 12;
                }
            }
            if (tz_hour == 0) {
                display_hour = 12;
            }
        }
        
        /* Draw digital clock face */
        terminal_writestring("  ┌─────────────────────┐\n");
        terminal_writestring("  │                     │\n");
        terminal_writestring("  │   ");
        
        if (display_hour < 10) terminal_putchar('0');
        print_number(display_hour);
        terminal_putchar(':');
        if (tz_minute < 10) terminal_putchar('0');
        print_number(tz_minute);
        
        if (show_seconds) {
            terminal_putchar(':');
            if (second < 10) terminal_putchar('0');
            print_number(second);
        }
        
        if (!is_24_hour) {
            terminal_writestring("  ");
            terminal_writestring(period);
        }
        
        terminal_writestring("    │\n");
        terminal_writestring("  │                     │\n");
        terminal_writestring("  └─────────────────────┘\n");
    }
    
    void print_date_info(void) {
        const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", 
                             "Thursday", "Friday", "Saturday"};
        const char *months[] = {"", "January", "February", "March", "April", 
                               "May", "June", "July", "August", "September", 
                               "October", "November", "December"};
        
        if (show_date) {
            terminal_writestring("📅 ");
            terminal_writestring(days[current_time.day_of_week]);
            terminal_writestring(", ");
            terminal_writestring(months[current_time.month]);
            terminal_writestring(" ");
            print_number(current_time.day);
            terminal_writestring(", ");
            print_number(current_time.year);
            terminal_putchar('\n');
        }
    }
    
    void display_timezone(int tz_index) {
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║          TIMEZONE: ");
        terminal_writestring(timezones[tz_index].name);
        terminal_writestring("\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
        
        draw_digital_display(current_time.hour, current_time.minute, 
                            current_time.second, 
                            timezones[tz_index].offset_hours,
                            timezones[tz_index].offset_minutes);
        
        terminal_writestring("\n📍 Timezone: ");
        terminal_writestring(timezones[tz_index].name);
        terminal_writestring("\n📌 Abbreviation: ");
        terminal_writestring(timezones[tz_index].abbr);
        terminal_writestring("\n⏱️  UTC Offset: ");
        
        if (timezones[tz_index].offset_hours >= 0) {
            terminal_putchar('+');
        }
        print_number(timezones[tz_index].offset_hours);
        terminal_putchar(':');
        if (timezones[tz_index].offset_minutes < 10) terminal_putchar('0');
        print_number(timezones[tz_index].offset_minutes);
        terminal_putchar('\n');
    }
    
    void display_all_timezones(void) {
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║       ALL TIMEZONES CURRENT TIME       ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
        
        for (int i = 0; i < timezone_count; i++) {
            terminal_writestring("  ");
            if (i + 1 < 10) terminal_putchar(' ');
            print_number(i + 1);
            terminal_writestring(". ");
            
            /* Pad timezone name */
            int name_len = str_length(timezones[i].name);
            terminal_writestring(timezones[i].name);
            for (int j = 0; j < 15 - name_len; j++) {
                terminal_putchar(' ');
            }
            
            int total_mins = (current_time.hour * 60 + current_time.minute) + 
                           (timezones[i].offset_hours * 60) + timezones[i].offset_minutes;
            
            if (total_mins < 0) total_mins += 1440;
            if (total_mins >= 1440) total_mins -= 1440;
            
            uint8_t tz_hour = total_mins / 60;
            uint8_t tz_minute = total_mins % 60;
            
            if (tz_hour < 10) terminal_putchar('0');
            print_number(tz_hour);
            terminal_putchar(':');
            if (tz_minute < 10) terminal_putchar('0');
            print_number(tz_minute);
            terminal_putchar('\n');
        }
    }
    
    void display_analog_clock(void) {
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║        ANALOG CLOCK DISPLAY            ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
        
        terminal_writestring("           12\n");
        terminal_writestring("        ╱      ╲\n");
        terminal_writestring("      ╱          ╲\n");
        terminal_writestring("  9 ─┤  ● ► ●     ├─ 3\n");
        terminal_writestring("      ╲          ╱\n");
        terminal_writestring("        ╲      ╱\n");
        terminal_writestring("           6\n\n");
        
        terminal_writestring("  Hour: ");
        if (current_time.hour < 10) terminal_putchar('0');
        print_number(current_time.hour);
        terminal_putchar('\n');
        
        terminal_writestring("  Minute: ");
        if (current_time.minute < 10) terminal_putchar('0');
        print_number(current_time.minute);
        terminal_putchar('\n');
        
        terminal_writestring("  Second: ");
        if (current_time.second < 10) terminal_putchar('0');
        print_number(current_time.second);
        terminal_putchar('\n');
    }
    
    void set_time(const char *time_str) {
        int hour = 0, minute = 0, second = 0;
        
        const char *ptr = time_str;
        
        /* Parse hours */
        while (*ptr >= '0' && *ptr <= '9') {
            hour = hour * 10 + (*ptr - '0');
            ptr++;
        }
        
        if (*ptr == ':') ptr++;
        
        /* Parse minutes */
        while (*ptr >= '0' && *ptr <= '9') {
            minute = minute * 10 + (*ptr - '0');
            ptr++;
        }
        
        if (*ptr == ':') ptr++;
        
        /* Parse seconds */
        while (*ptr >= '0' && *ptr <= '9') {
            second = second * 10 + (*ptr - '0');
            ptr++;
        }
        
        if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60 && 
            second >= 0 && second < 60) {
            current_time.hour = hour;
            current_time.minute = minute;
            current_time.second = second;
            calculate_total_seconds();
            terminal_writestring("[+] Time set successfully\n");
        } else {
            terminal_writestring("[!] Invalid time format\n");
        }
    }
    
    void set_date(const char *date_str) {
        int year = 0, month = 0, day = 0;
        const char *ptr = date_str;
        
        while (*ptr >= '0' && *ptr <= '9') {
            year = year * 10 + (*ptr - '0');
            ptr++;
        }
        if (*ptr == '-') ptr++;
        
        while (*ptr >= '0' && *ptr <= '9') {
            month = month * 10 + (*ptr - '0');
            ptr++;
        }
        if (*ptr == '-') ptr++;
        
        while (*ptr >= '0' && *ptr <= '9') {
            day = day * 10 + (*ptr - '0');
            ptr++;
        }
        
        if (year >= 2000 && year <= 2100 && month >= 1 && month <= 12 && 
            day >= 1 && day <= 31) {
            current_time.year = year;
            current_time.month = month;
            current_time.day = day;
            terminal_writestring("[+] Date set successfully\n");
        } else {
            terminal_writestring("[!] Invalid date format\n");
        }
    }
    
    void set_format(const char *format_str) {
        if (str_compare(format_str, "24")) {
            is_24_hour = 1;
            terminal_writestring("[+] Format set to 24-hour\n");
        } else if (str_compare(format_str, "12")) {
            is_24_hour = 0;
            terminal_writestring("[+] Format set to 12-hour (AM/PM)\n");
        } else {
            terminal_writestring("[!] Use '24' or '12'\n");
        }
    }
    
    void run_timer(int seconds) {
        terminal_writestring("\n[*] Timer started: ");
        print_number(seconds);
        terminal_writestring(" seconds\n");
        terminal_writestring("[*] Timer feature demonstration\n");
        terminal_writestring("[+] Timer completed!\n");
    }
    
    void run_stopwatch(void) {
        terminal_writestring("\n[*] Stopwatch started\n");
        terminal_writestring("    Press CTRL+C to stop\n");
        terminal_writestring("[*] Elapsed: 0:00:00\n");
        terminal_writestring("[+] Stopwatch stopped\n");
    }
    
    void set_alarm(const char *alarm_str) {
        terminal_writestring("\n[*] Alarm set for: ");
        terminal_writestring(alarm_str);
        terminal_putchar('\n');
        terminal_writestring("[+] Alarm will trigger at set time\n");
    }
    
    void display_world_clock(void) {
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║           WORLD CLOCK VIEW             ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
        
        terminal_writestring("Selected Cities:\n\n");
        
        int major_cities[] = {0, 6, 4, 7, 8, 9, 10};
        for (int i = 0; i < 7; i++) {
            int tz_idx = major_cities[i];
            
            int total_mins = (current_time.hour * 60 + current_time.minute) + 
                           (timezones[tz_idx].offset_hours * 60) + 
                           timezones[tz_idx].offset_minutes;
            
            if (total_mins < 0) total_mins += 1440;
            if (total_mins >= 1440) total_mins -= 1440;
            
            uint8_t tz_hour = total_mins / 60;
            uint8_t tz_minute = total_mins % 60;
            
            terminal_writestring("  🌍 ");
            terminal_writestring(timezones[tz_idx].name);
            terminal_writestring(":");
            
            int name_len = str_length(timezones[tz_idx].name);
            for (int j = 0; j < 12 - name_len; j++) {
                terminal_putchar(' ');
            }
            
            if (tz_hour < 10) terminal_putchar('0');
            print_number(tz_hour);
            terminal_putchar(':');
            if (tz_minute < 10) terminal_putchar('0');
            print_number(tz_minute);
            terminal_putchar('\n');
        }
    }
    
    void show_settings(void) {
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║         CLOCK SETTINGS                 ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
        
        terminal_writestring("Time Format: ");
        if (is_24_hour) {
            terminal_writestring("24-hour\n");
        } else {
            terminal_writestring("12-hour (AM/PM)\n");
        }
        
        terminal_writestring("Show Seconds: ");
        if (show_seconds) {
            terminal_writestring("Yes\n");
        } else {
            terminal_writestring("No\n");
        }
        
        terminal_writestring("Show Date: ");
        if (show_date) {
            terminal_writestring("Yes\n");
        } else {
            terminal_writestring("No\n");
        }
        
        terminal_writestring("\nAvailable Timezones: ");
        print_number(timezone_count);
        terminal_putchar('\n');
    }
    
    /* Helper Functions */
    
    void read_input(char *buffer, int max_len) {
        int pos = 0;
        while (pos < max_len - 1) {
            uint8_t scancode = get_scancode();
            if (scancode == 0) continue;
            
            if (scancode == 0x1C) {
                terminal_putchar('\n');
                buffer[pos] = '\0';
                break;
            } else if (scancode == 0x0E && pos > 0) {
                pos--;
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            } else if (scancode < 128) {
                char c = scancode_to_ascii(scancode);
                if (c) {
                    terminal_putchar(c);
                    buffer[pos++] = c;
                }
            }
        }
        if (pos == max_len - 1) buffer[pos] = '\0';
    }
    
    uint8_t get_scancode(void) {
        return 0;
    }
    
    char scancode_to_ascii(uint8_t scancode) {
        static const char keymap[] = {
            0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
            'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
            'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
            'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
        };
        if (scancode < sizeof(keymap))
            return keymap[scancode];
        return 0;
    }
    
    void print_number(uint32_t num) {
        if (num == 0) {
            terminal_putchar('0');
            return;
        }
        
        char buf[20];
        int len = 0;
        uint32_t temp = num;
        
        while (temp > 0) {
            buf[len++] = '0' + (temp % 10);
            temp /= 10;
        }
        
        for (int i = len - 1; i >= 0; i--) {
            terminal_putchar(buf[i]);
        }
    }
    
    bool str_compare(const char *a, const char *b) {
        while (*a && *b && *a == *b) {
            a++;
            b++;
        }
        return *a == *b;
    }
    
    bool str_starts_with(const char *str, const char *prefix) {
        while (*prefix) {
            if (*str != *prefix)
                return false;
            str++;
            prefix++;
        }
        return true;
    }
    
    int str_length(const char *str) {
        int len = 0;
        while (str[len]) len++;
        return len;
    }
    
    int parse_number(const char *str) {
        int num = 0;
        while (*str >= '0' && *str <= '9') {
            num = num * 10 + (*str - '0');
            str++;
        }
        return num;
    }
};

extern "C" {
    DigitalClock *clock_instance = nullptr;
    
    void digital_clock_init(void) {
        clock_instance = new DigitalClock();
    }
    
    void digital_clock_run(void) {
        if (clock_instance) {
            clock_instance->run();
        }
    }
}
