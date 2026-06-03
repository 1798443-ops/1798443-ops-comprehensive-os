#include <cstdint>
#include <cstring>
#include <cstdlib>

extern "C" {
    void terminal_putchar(char c);
    void terminal_writestring(const char *str);
    void terminal_clear(void);
}

/* To-Do List Application with Local Storage */
#define MAX_TASKS 100
#define MAX_TASK_LENGTH 256
#define MAX_CATEGORIES 20
#define STORAGE_SIZE 16384

typedef struct {
    char title[MAX_TASK_LENGTH];
    char description[MAX_TASK_LENGTH];
    char category[64];
    uint8_t priority; /* 1=Low, 2=Medium, 3=High */
    uint8_t completed;
    uint32_t created_time;
    uint32_t due_date;
    uint8_t recurring; /* 0=No, 1=Daily, 2=Weekly, 3=Monthly */
} task_t;

typedef struct {
    char name[64];
    uint32_t task_count;
} category_t;

class TodoListApp {
private:
    task_t tasks[MAX_TASKS];
    category_t categories[MAX_CATEGORIES];
    uint32_t task_count;
    uint32_t category_count;
    
    /* Local Storage */
    uint8_t storage[STORAGE_SIZE];
    uint32_t storage_pos;
    
    /* UI State */
    uint8_t sort_by; /* 0=Priority, 1=Date, 2=Completed */
    uint8_t filter_category; /* 255=All */
    uint8_t show_completed;
    
public:
    TodoListApp() : task_count(0), category_count(0), storage_pos(0),
                    sort_by(0), filter_category(255), show_completed(1) {
        memset(tasks, 0, sizeof(tasks));
        memset(categories, 0, sizeof(categories));
        memset(storage, 0, sizeof(storage));
        
        /* Initialize default categories */
        init_categories();
        load_tasks_from_storage();
    }
    
    void run(void) {
        terminal_clear();
        print_banner();
        print_main_menu();
        
        char command[256];
        while (1) {
            terminal_writestring("\ntodo> ");
            read_input(command, 256);
            
            if (!execute_command(command)) {
                break;
            }
        }
    }
    
private:
    void init_categories(void) {
        const char *default_cats[] = {
            "Work", "Personal", "Shopping", "Health", 
            "Finance", "Education", "Home", "Project"
        };
        
        category_count = 8;
        for (int i = 0; i < 8; i++) {
            str_copy(categories[i].name, default_cats[i], 64);
            categories[i].task_count = 0;
        }
    }
    
    void print_banner(void) {
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║   ✓ TODO LIST APPLICATION v1.0        ║\n");
        terminal_writestring("║     Task Management System             ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
    }
    
    void print_main_menu(void) {
        terminal_writestring("=== MAIN MENU ===\n");
        terminal_writestring("  add              - Add new task\n");
        terminal_writestring("  list             - List all tasks\n");
        terminal_writestring("  view <id>        - View task details\n");
        terminal_writestring("  edit <id>        - Edit task\n");
        terminal_writestring("  complete <id>    - Mark task complete\n");
        terminal_writestring("  delete <id>      - Delete task\n");
        terminal_writestring("  search <text>    - Search tasks\n");
        terminal_writestring("  filter <cat>     - Filter by category\n");
        terminal_writestring("  sort <type>      - Sort (priority/date/status)\n");
        terminal_writestring("  categories       - Manage categories\n");
        terminal_writestring("  statistics       - Show statistics\n");
        terminal_writestring("  export           - Export tasks\n");
        terminal_writestring("  import           - Import tasks\n");
        terminal_writestring("  clear-storage    - Clear all data\n");
        terminal_writestring("  help             - Show this menu\n");
        terminal_writestring("  exit             - Exit application\n");
    }
    
    bool execute_command(const char *cmd) {
        if (cmd[0] == '\0') return true;
        
        if (str_compare(cmd, "add")) {
            add_task();
            return true;
        }
        else if (str_compare(cmd, "list")) {
            display_tasks();
            return true;
        }
        else if (str_starts_with(cmd, "view ")) {
            int id = parse_number(cmd + 5);
            view_task(id);
            return true;
        }
        else if (str_starts_with(cmd, "edit ")) {
            int id = parse_number(cmd + 5);
            edit_task(id);
            return true;
        }
        else if (str_starts_with(cmd, "complete ")) {
            int id = parse_number(cmd + 9);
            complete_task(id);
            return true;
        }
        else if (str_starts_with(cmd, "delete ")) {
            int id = parse_number(cmd + 7);
            delete_task(id);
            return true;
        }
        else if (str_starts_with(cmd, "search ")) {
            search_tasks(cmd + 7);
            return true;
        }
        else if (str_starts_with(cmd, "filter ")) {
            filter_by_category(cmd + 7);
            return true;
        }
        else if (str_starts_with(cmd, "sort ")) {
            set_sort_method(cmd + 5);
            return true;
        }
        else if (str_compare(cmd, "categories")) {
            manage_categories();
            return true;
        }
        else if (str_compare(cmd, "statistics")) {
            show_statistics();
            return true;
        }
        else if (str_compare(cmd, "export")) {
            export_tasks();
            return true;
        }
        else if (str_compare(cmd, "import")) {
            import_tasks();
            return true;
        }
        else if (str_compare(cmd, "clear-storage")) {
            clear_storage();
            return true;
        }
        else if (str_compare(cmd, "help")) {
            print_main_menu();
            return true;
        }
        else if (str_compare(cmd, "exit") || str_compare(cmd, "quit")) {
            terminal_writestring("\n[*] Saving tasks...\n");
            save_tasks_to_storage();
            terminal_writestring("[+] Tasks saved. Exiting...\n");
            return false;
        }
        else {
            terminal_writestring("Unknown command. Type 'help' for options.\n");
            return true;
        }
    }
    
    void add_task(void) {
        if (task_count >= MAX_TASKS) {
            terminal_writestring("[!] Maximum tasks reached\n");
            return;
        }
        
        terminal_writestring("\n=== ADD NEW TASK ===\n\n");
        
        terminal_writestring("Task title: ");
        char title[MAX_TASK_LENGTH];
        read_input(title, MAX_TASK_LENGTH);
        
        terminal_writestring("Description (optional): ");
        char description[MAX_TASK_LENGTH];
        read_input(description, MAX_TASK_LENGTH);
        
        terminal_writestring("\nCategories:\n");
        for (uint32_t i = 0; i < category_count; i++) {
            terminal_writestring("  ");
            print_number(i + 1);
            terminal_writestring(". ");
            terminal_writestring(categories[i].name);
            terminal_putchar('\n');
        }
        
        terminal_writestring("Select category (1-");
        print_number(category_count);
        terminal_writestring("): ");
        char cat_choice[10];
        read_input(cat_choice, 10);
        int cat_idx = parse_number(cat_choice) - 1;
        
        if (cat_idx < 0 || cat_idx >= (int)category_count) {
            cat_idx = 0;
        }
        
        terminal_writestring("Priority (1=Low, 2=Medium, 3=High): ");
        char priority_str[10];
        read_input(priority_str, 10);
        int priority = parse_number(priority_str);
        if (priority < 1 || priority > 3) priority = 2;
        
        terminal_writestring("Recurring (0=No, 1=Daily, 2=Weekly, 3=Monthly): ");
        char recurring_str[10];
        read_input(recurring_str, 10);
        int recurring = parse_number(recurring_str);
        if (recurring < 0 || recurring > 3) recurring = 0;
        
        /* Create task */
        task_t new_task;
        str_copy(new_task.title, title, MAX_TASK_LENGTH);
        str_copy(new_task.description, description, MAX_TASK_LENGTH);
        str_copy(new_task.category, categories[cat_idx].name, 64);
        new_task.priority = priority;
        new_task.completed = 0;
        new_task.created_time = 0;
        new_task.due_date = 0;
        new_task.recurring = recurring;
        
        tasks[task_count] = new_task;
        task_count++;
        categories[cat_idx].task_count++;
        
        terminal_writestring("\n[+] Task added successfully! (ID: ");
        print_number(task_count);
        terminal_writestring(")\n");
        
        save_tasks_to_storage();
    }
    
    void display_tasks(void) {
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════════════════════════╗\n");
        terminal_writestring("║                   TASK LIST                               ║\n");
        terminal_writestring("╚════════════════════════════════════════════════════════════╝\n\n");
        
        if (task_count == 0) {
            terminal_writestring("No tasks available. Add one with 'add' command.\n");
            return;
        }
        
        terminal_writestring("ID  │ Title                      │ Status    │ Priority │ Category\n");
        terminal_writestring("────┼────────────────────────────┼───────────┼──────────┼──────────\n");
        
        for (uint32_t i = 0; i < task_count; i++) {
            if (!show_completed && tasks[i].completed) continue;
            if (filter_category != 255 && 
                !str_compare(tasks[i].category, categories[filter_category].name)) {
                continue;
            }
            
            print_number(i + 1);
            terminal_writestring("  │ ");
            
            /* Print title (truncated) */
            int title_len = str_length(tasks[i].title);
            int print_len = title_len > 26 ? 26 : title_len;
            for (int j = 0; j < print_len; j++) {
                terminal_putchar(tasks[i].title[j]);
            }
            for (int j = print_len; j < 26; j++) {
                terminal_putchar(' ');
            }
            terminal_writestring(" │ ");
            
            /* Status */
            if (tasks[i].completed) {
                terminal_writestring("✓ Complete");
            } else {
                terminal_writestring("○ Pending ");
            }
            terminal_writestring(" │ ");
            
            /* Priority */
            switch (tasks[i].priority) {
                case 1: terminal_writestring("Low     "); break;
                case 2: terminal_writestring("Medium  "); break;
                case 3: terminal_writestring("High    "); break;
            }
            terminal_writestring(" │ ");
            
            /* Category */
            terminal_writestring(tasks[i].category);
            terminal_putchar('\n');
        }
        
        terminal_writestring("\nTotal tasks: ");
        print_number(task_count);
        terminal_writestring(" | Completed: ");
        int completed = 0;
        for (uint32_t i = 0; i < task_count; i++) {
            if (tasks[i].completed) completed++;
        }
        print_number(completed);
        terminal_putchar('\n');
    }
    
    void view_task(int id) {
        if (id < 1 || id > (int)task_count) {
            terminal_writestring("[!] Invalid task ID\n");
            return;
        }
        
        task_t *task = &tasks[id - 1];
        
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║           TASK DETAILS                 ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
        
        terminal_writestring("Task ID: ");
        print_number(id);
        terminal_putchar('\n');
        
        terminal_writestring("Title: ");
        terminal_writestring(task->title);
        terminal_putchar('\n');
        
        terminal_writestring("Description: ");
        if (task->description[0] != '\0') {
            terminal_writestring(task->description);
        } else {
            terminal_writestring("(None)");
        }
        terminal_putchar('\n');
        
        terminal_writestring("Category: ");
        terminal_writestring(task->category);
        terminal_putchar('\n');
        
        terminal_writestring("Priority: ");
        switch (task->priority) {
            case 1: terminal_writestring("Low"); break;
            case 2: terminal_writestring("Medium"); break;
            case 3: terminal_writestring("High"); break;
        }
        terminal_putchar('\n');
        
        terminal_writestring("Status: ");
        if (task->completed) {
            terminal_writestring("✓ Completed");
        } else {
            terminal_writestring("○ Pending");
        }
        terminal_putchar('\n');
        
        terminal_writestring("Recurring: ");
        switch (task->recurring) {
            case 0: terminal_writestring("No"); break;
            case 1: terminal_writestring("Daily"); break;
            case 2: terminal_writestring("Weekly"); break;
            case 3: terminal_writestring("Monthly"); break;
        }
        terminal_putchar('\n');
    }
    
    void edit_task(int id) {
        if (id < 1 || id > (int)task_count) {
            terminal_writestring("[!] Invalid task ID\n");
            return;
        }
        
        task_t *task = &tasks[id - 1];
        
        terminal_writestring("\n=== EDIT TASK ===\n\n");
        terminal_writestring("Current title: ");
        terminal_writestring(task->title);
        terminal_writestring("\nNew title (enter to skip): ");
        
        char new_title[MAX_TASK_LENGTH];
        read_input(new_title, MAX_TASK_LENGTH);
        if (new_title[0] != '\0') {
            str_copy(task->title, new_title, MAX_TASK_LENGTH);
        }
        
        terminal_writestring("New description (enter to skip): ");
        char new_desc[MAX_TASK_LENGTH];
        read_input(new_desc, MAX_TASK_LENGTH);
        if (new_desc[0] != '\0') {
            str_copy(task->description, new_desc, MAX_TASK_LENGTH);
        }
        
        terminal_writestring("New priority (1-3, enter to skip): ");
        char priority_str[10];
        read_input(priority_str, 10);
        if (priority_str[0] != '\0') {
            int priority = parse_number(priority_str);
            if (priority >= 1 && priority <= 3) {
                task->priority = priority;
            }
        }
        
        terminal_writestring("[+] Task updated successfully\n");
        save_tasks_to_storage();
    }
    
    void complete_task(int id) {
        if (id < 1 || id > (int)task_count) {
            terminal_writestring("[!] Invalid task ID\n");
            return;
        }
        
        tasks[id - 1].completed = 1;
        terminal_writestring("[+] Task marked as complete\n");
        save_tasks_to_storage();
    }
    
    void delete_task(int id) {
        if (id < 1 || id > (int)task_count) {
            terminal_writestring("[!] Invalid task ID\n");
            return;
        }
        
        terminal_writestring("Are you sure? (y/n): ");
        char response[10];
        read_input(response, 10);
        
        if (response[0] == 'y' || response[0] == 'Y') {
            /* Shift tasks */
            for (int i = id - 1; i < (int)task_count - 1; i++) {
                tasks[i] = tasks[i + 1];
            }
            task_count--;
            terminal_writestring("[+] Task deleted\n");
            save_tasks_to_storage();
        } else {
            terminal_writestring("[*] Cancelled\n");
        }
    }
    
    void search_tasks(const char *search_term) {
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║         SEARCH RESULTS                 ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
        
        int results = 0;
        for (uint32_t i = 0; i < task_count; i++) {
            if (str_contains(tasks[i].title, search_term) || 
                str_contains(tasks[i].description, search_term)) {
                terminal_writestring("ID: ");
                print_number(i + 1);
                terminal_writestring(" │ ");
                terminal_writestring(tasks[i].title);
                terminal_putchar('\n');
                results++;
            }
        }
        
        if (results == 0) {
            terminal_writestring("No tasks found.\n");
        } else {
            terminal_writestring("\nFound: ");
            print_number(results);
            terminal_writestring(" task(s)\n");
        }
    }
    
    void filter_by_category(const char *cat_name) {
        uint32_t found_cat = 255;
        for (uint32_t i = 0; i < category_count; i++) {
            if (str_compare(categories[i].name, cat_name)) {
                found_cat = i;
                break;
            }
        }
        
        if (found_cat == 255) {
            terminal_writestring("[!] Category not found\n");
            return;
        }
        
        filter_category = found_cat;
        terminal_writestring("[+] Filtering by: ");
        terminal_writestring(cat_name);
        terminal_putchar('\n');
        display_tasks();
    }
    
    void set_sort_method(const char *method) {
        if (str_compare(method, "priority")) {
            sort_by = 0;
            terminal_writestring("[+] Sorting by priority\n");
        } else if (str_compare(method, "date")) {
            sort_by = 1;
            terminal_writestring("[+] Sorting by date\n");
        } else if (str_compare(method, "status")) {
            sort_by = 2;
            terminal_writestring("[+] Sorting by status\n");
        } else {
            terminal_writestring("[!] Unknown sort method\n");
        }
    }
    
    void manage_categories(void) {
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║      CATEGORY MANAGEMENT               ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
        
        terminal_writestring("Current categories:\n");
        for (uint32_t i = 0; i < category_count; i++) {
            terminal_writestring("  ");
            print_number(i + 1);
            terminal_writestring(". ");
            terminal_writestring(categories[i].name);
            terminal_writestring(" (");
            print_number(categories[i].task_count);
            terminal_writestring(" tasks)\n");
        }
        
        terminal_writestring("\nOptions:\n");
        terminal_writestring("  1. Add category\n");
        terminal_writestring("  2. Delete category\n");
        terminal_writestring("  3. Rename category\n");
        terminal_writestring("  4. Back\n\n");
        
        terminal_writestring("Choose option: ");
        char choice[10];
        read_input(choice, 10);
        
        switch (choice[0]) {
            case '1': {
                if (category_count >= MAX_CATEGORIES) {
                    terminal_writestring("[!] Max categories reached\n");
                    break;
                }
                terminal_writestring("New category name: ");
                char cat_name[64];
                read_input(cat_name, 64);
                str_copy(categories[category_count].name, cat_name, 64);
                categories[category_count].task_count = 0;
                category_count++;
                terminal_writestring("[+] Category added\n");
                break;
            }
            case '2': {
                terminal_writestring("Delete category number: ");
                char del_num[10];
                read_input(del_num, 10);
                int del_idx = parse_number(del_num) - 1;
                if (del_idx >= 0 && del_idx < (int)category_count) {
                    for (int i = del_idx; i < (int)category_count - 1; i++) {
                        categories[i] = categories[i + 1];
                    }
                    category_count--;
                    terminal_writestring("[+] Category deleted\n");
                }
                break;
            }
            case '3': {
                terminal_writestring("Rename category number: ");
                char rename_num[10];
                read_input(rename_num, 10);
                int rename_idx = parse_number(rename_num) - 1;
                if (rename_idx >= 0 && rename_idx < (int)category_count) {
                    terminal_writestring("New name: ");
                    char new_name[64];
                    read_input(new_name, 64);
                    str_copy(categories[rename_idx].name, new_name, 64);
                    terminal_writestring("[+] Category renamed\n");
                }
                break;
            }
        }
    }
    
    void show_statistics(void) {
        terminal_writestring("\n");
        terminal_writestring("╔════════════════════════════════════════╗\n");
        terminal_writestring("║         TASK STATISTICS                ║\n");
        terminal_writestring("╚════════════════════════════════════════╝\n\n");
        
        int completed = 0, pending = 0, high = 0, medium = 0, low = 0;
        
        for (uint32_t i = 0; i < task_count; i++) {
            if (tasks[i].completed) completed++;
            else pending++;
            
            switch (tasks[i].priority) {
                case 1: low++; break;
                case 2: medium++; break;
                case 3: high++; break;
            }
        }
        
        terminal_writestring("Total Tasks: ");
        print_number(task_count);
        terminal_putchar('\n');
        
        terminal_writestring("Completed: ");
        print_number(completed);
        terminal_writestring(" (");
        if (task_count > 0) {
            print_number((completed * 100) / task_count);
        } else {
            terminal_putchar('0');
        }
        terminal_writestring("%)\n");
        
        terminal_writestring("Pending: ");
        print_number(pending);
        terminal_putchar('\n');
        
        terminal_writestring("\nBy Priority:\n");
        terminal_writestring("  High: ");
        print_number(high);
        terminal_putchar('\n');
        terminal_writestring("  Medium: ");
        print_number(medium);
        terminal_putchar('\n');
        terminal_writestring("  Low: ");
        print_number(low);
        terminal_putchar('\n');
        
        terminal_writestring("\nBy Category:\n");
        for (uint32_t i = 0; i < category_count; i++) {
            terminal_writestring("  ");
            terminal_writestring(categories[i].name);
            terminal_writestring(": ");
            print_number(categories[i].task_count);
            terminal_putchar('\n');
        }
    }
    
    void export_tasks(void) {
        terminal_writestring("\n[*] Exporting tasks to storage...\n");
        save_tasks_to_storage();
        terminal_writestring("[+] Tasks exported successfully\n");
        terminal_writestring("[+] Total tasks: ");
        print_number(task_count);
        terminal_putchar('\n');
    }
    
    void import_tasks(void) {
        terminal_writestring("\n[*] Importing tasks from storage...\n");
        load_tasks_from_storage();
        terminal_writestring("[+] Tasks imported successfully\n");
        terminal_writestring("[+] Total tasks: ");
        print_number(task_count);
        terminal_putchar('\n');
    }
    
    void clear_storage(void) {
        terminal_writestring("Are you sure? This will delete ALL tasks (y/n): ");
        char response[10];
        read_input(response, 10);
        
        if (response[0] == 'y' || response[0] == 'Y') {
            task_count = 0;
            memset(tasks, 0, sizeof(tasks));
            memset(storage, 0, sizeof(storage));
            storage_pos = 0;
            terminal_writestring("[+] All tasks cleared\n");
        } else {
            terminal_writestring("[*] Cancelled\n");
        }
    }
    
    /* Storage Functions */
    
    void save_tasks_to_storage(void) {
        storage_pos = 0;
        
        /* Save task count */
        storage[storage_pos++] = (task_count >> 24) & 0xFF;
        storage[storage_pos++] = (task_count >> 16) & 0xFF;
        storage[storage_pos++] = (task_count >> 8) & 0xFF;
        storage[storage_pos++] = task_count & 0xFF;
        
        /* Save each task */
        for (uint32_t i = 0; i < task_count && storage_pos < STORAGE_SIZE - 1024; i++) {
            task_t *task = &tasks[i];
            
            /* Save title length and content */
            int title_len = str_length(task->title);
            storage[storage_pos++] = title_len & 0xFF;
            for (int j = 0; j < title_len; j++) {
                storage[storage_pos++] = task->title[j];
            }
            
            /* Save description */
            int desc_len = str_length(task->description);
            storage[storage_pos++] = desc_len & 0xFF;
            for (int j = 0; j < desc_len; j++) {
                storage[storage_pos++] = task->description[j];
            }
            
            /* Save category */
            int cat_len = str_length(task->category);
            storage[storage_pos++] = cat_len & 0xFF;
            for (int j = 0; j < cat_len; j++) {
                storage[storage_pos++] = task->category[j];
            }
            
            /* Save flags */
            storage[storage_pos++] = task->priority;
            storage[storage_pos++] = task->completed;
            storage[storage_pos++] = task->recurring;
        }
    }
    
    void load_tasks_from_storage(void) {
        storage_pos = 0;
        
        if (storage_pos + 4 > STORAGE_SIZE) return;
        
        /* Load task count */
        task_count = (storage[storage_pos] << 24) | 
                    (storage[storage_pos+1] << 16) | 
                    (storage[storage_pos+2] << 8) | 
                    storage[storage_pos+3];
        storage_pos += 4;
        
        if (task_count > MAX_TASKS) task_count = 0;
        
        /* Load each task */
        for (uint32_t i = 0; i < task_count; i++) {
            task_t *task = &tasks[i];
            
            /* Load title */
            if (storage_pos >= STORAGE_SIZE) break;
            int title_len = storage[storage_pos++];
            for (int j = 0; j < title_len && storage_pos < STORAGE_SIZE; j++) {
                task->title[j] = storage[storage_pos++];
            }
            task->title[title_len] = '\0';
            
            /* Load description */
            if (storage_pos >= STORAGE_SIZE) break;
            int desc_len = storage[storage_pos++];
            for (int j = 0; j < desc_len && storage_pos < STORAGE_SIZE; j++) {
                task->description[j] = storage[storage_pos++];
            }
            task->description[desc_len] = '\0';
            
            /* Load category */
            if (storage_pos >= STORAGE_SIZE) break;
            int cat_len = storage[storage_pos++];
            for (int j = 0; j < cat_len && storage_pos < STORAGE_SIZE; j++) {
                task->category[j] = storage[storage_pos++];
            }
            task->category[cat_len] = '\0';
            
            /* Load flags */
            if (storage_pos + 3 > STORAGE_SIZE) break;
            task->priority = storage[storage_pos++];
            task->completed = storage[storage_pos++];
            task->recurring = storage[storage_pos++];
        }
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
    
    bool str_contains(const char *str, const char *search) {
        while (*str) {
            const char *s1 = str;
            const char *s2 = search;
            while (*s1 && *s2 && *s1 == *s2) {
                s1++;
                s2++;
            }
            if (*s2 == '\0')
                return true;
            str++;
        }
        return false;
    }
    
    int str_length(const char *str) {
        int len = 0;
        while (str[len]) len++;
        return len;
    }
    
    void str_copy(char *dst, const char *src, int max_len) {
        int i = 0;
        while (src[i] && i < max_len - 1) {
            dst[i] = src[i];
            i++;
        }
        dst[i] = '\0';
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
    TodoListApp *todo_instance = nullptr;
    
    void todo_list_init(void) {
        todo_instance = new TodoListApp();
    }
    
    void todo_list_run(void) {
        if (todo_instance) {
            todo_instance->run();
        }
    }
}
