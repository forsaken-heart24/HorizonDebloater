#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

#define BUFFER_SIZE 256
#define MAX_PACKAGES 200

bool __is__powershell() {
    return getenv("PSExecutionPolicy") != NULL;
}

int adb_executable(char *app_argument, char *arguments) {
    if (arguments == NULL) {
        fprintf(stderr, "Error: Missing required arguments.\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(app_argument, "wait-for-device") != 0 && app_argument == NULL) {
        fprintf(stderr, "Error: Missing required arguments.\n");
        exit(EXIT_FAILURE);
    }

    FILE *adb_executable_file = popen(".\\bin\\adb.exe", "r");
    char package_command[512];

    if (adb_executable_file == NULL) {
        if (__is__powershell()) {
            snprintf(package_command, sizeof(package_command), ".\\bin\\adb.exe %s %s | out-null", app_argument, arguments);
        } else {
            snprintf(package_command, sizeof(package_command), ".\\bin\\adb.exe %s %s > nul 2>&1", app_argument, arguments);
        }
    } else {
        if (__is__powershell()) {
            snprintf(package_command, sizeof(package_command), "adb %s %s | out-null", app_argument, arguments);
        } else {
            snprintf(package_command, sizeof(package_command), "adb %s %s > nul 2>&1", app_argument, arguments);
        }
        fclose(adb_executable_file);
    }

    return system(package_command) == 0 ? 0 : 1;
}

void print_centered(const char *text) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int terminal_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int text_length = strlen(text);
    int padding = (terminal_width - text_length) / 2;

    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    printf("%s\n", text);
}

void animate_ascii_art(const char *ascii_art[], int lines_count) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int terminal_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    system("cls");
    for (int i = terminal_height; i >= 0; i--) {
        system("cls");
        for (int j = 0; j < i; j++) {
            printf("\n");
        }
        for (int k = 0; k < lines_count; k++) {
            print_centered(ascii_art[k]);
        }
        Sleep(100);
    }
}

const char* get_system_property(const char *property) {
    static char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "adb shell getprop %s", property);
    FILE *fp = popen(buffer, "r");
    if (fp == NULL) {
        return NULL;
    }
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        pclose(fp);
        return buffer;
    } else {
        snprintf(buffer, sizeof(buffer), ".\\bin\\adb.exe shell getprop %s", property);
        pclose(fp);
        return buffer;
    }
}

void uninstall_package(const char *package_name) {
    char error_message[BUFFER_SIZE];
    char package_command[BUFFER_SIZE];

    snprintf(package_command, sizeof(package_command), "pm uninstall --user 0 %s", package_name);

    snprintf(error_message, sizeof(error_message), "\033[0;36mUninstalling %s package", package_name);
    print_centered(error_message);

    if (adb_executable("shell", package_command) != 0) {
        snprintf(error_message, sizeof(error_message), "\033[0;31mFailed to uninstall: %s", package_name);
        print_centered(error_message);
    } else {
        snprintf(error_message, sizeof(error_message), "\033[0;32mSuccessfully uninstalled %s", package_name);
        print_centered(error_message);
    }
}

void get_device_serial(char *serial, size_t size) {
    FILE *fp = popen("adb shell getprop ro.serialno", "r");
    if (fp == NULL) {
        snprintf(serial, size, "Unknown");
        return;
    }
    if (fgets(serial, size, fp) != NULL) {
        serial[strcspn(serial, "\n")] = 0;
        char sanitized[BUFFER_SIZE] = {0};
        size_t j = 0;
        for (size_t i = 0; serial[i] != '\0'; i++) {
            if (isalnum((unsigned char)serial[i])) {
                sanitized[j++] = serial[i];
            }
        }
        sanitized[j] = '\0';
        snprintf(serial, size, "%s", sanitized);
    }
    pclose(fp);
}

int load_debloat_list(const char *filename, char debloat_list[][BUFFER_SIZE], int max_packages) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open debloat list file");
        return 0;
    }

    int count = 0;
    while (count < max_packages && fgets(debloat_list[count], BUFFER_SIZE, file)) {
        debloat_list[count][strcspn(debloat_list[count], "\n")] = 0;
        count++;
    }
    fclose(file);
    return count;
}

int main(int argc, char *argv[]) {
    char debloat_list[MAX_PACKAGES][BUFFER_SIZE];
    FILE *adb_exe = popen(".\\bin\\adb", "r");
    int package_count = 0;

    const char *default_debloat_list[] = {
        "com.miui.analytics", "com.google.android.projection.gearhead", "com.android.egg", 
        "com.android.bluetoothmidiservice", "com.android.bookmarkprovider", "com.google.android.cellbroadcastservice", 
        "com.miui.cit", "com.android.cellbroadcastreceiver", "com.android.cellbroadcastreceiver.overlay.common",
        "com.android.cts.priv.ctsshim", "com.android.providers.partnerbookmarks", "com.android.wallpaperbackup", 
        "com.android.wallpapercropper", "com.google.android.cellbroadcastreceiver.overlay.miui", 
        "com.google.android.cellbroadcastservice.overlay.miui", "com.qualcomm.uimremoteclient", 
        "com.qualcomm.uimremoteserver", "com.google.android.apps.turbo", "com.facebook.system", 
        "com.facebook.appmanager", "com.facebook.services", "com.xiaomi.mipicks", "com.google.android.gm", 
        "com.google.android.googlequicksearchbox", "com.android.hotwordenrollment.xgoogle", 
        "com.android.hotwordenrollment.okgoogle", "com.google.android.apps.subscriptions.red", 
        "com.google.android.onetimeinitializer", "com.google.android.tts", "com.qualcomm.location", 
        "com.xiaomi.payment", "com.xiaomi.mi_connect_service", "com.miui.mishare.connectivity", 
        "com.miui.videoplayer", "com.miui.daemon", "com.android.mms.service", "com.miui.msa.global", 
        "com.android.theme.font.notoserifsource", "com.google.android.printservice.recommendation", 
        "com.miui.hybrid", "com.miui.audiomonitor", "com.miui.hybrid.accessory", "com.qti.xdivert", 
        "com.tencent.soter.soterserver", "com.xiaomi.xmsfkeeper", "com.google.android.apps.googleassistant", 
        "com.android.emergency", "com.google.android.gms.location.history", "com.google.android.apps.maps", 
        "com.android.wallpaper.livepicker", "com.android.stk", "com.miui.player", "com.google.android.partnersetup", 
        "com.google.android.feedback", "com.miui.bugreport", "com.miui.touchassistant", "com.android.bips", 
        "com.miui.miservice", "com.google.android.marvin.talkback", "com.miui.yellowpage", "com.android.printspooler", 
        "com.milink.service", "com.android.traceur", "com.google.android.apps.wellbeing", "com.miui.phrase", 
        "com.google.android.cellbroadcastreceiver", "com.android.internal.systemui.navbar.threebutton", 
        "com.android.systemui.navigation.bar.overlay", "com.android.internal.systemui.navbar.gestural", 
        "com.android.internal.systemui.navbar.gestural_wide_back", "com.android.internal.systemui.navbar.gestural_extra_wide_back", 
        "com.android.internal.systemui.navbar.gestural_narrow_back", "com.xiaomi.gnss.polaris", 
        "com.android.smspush", "com.android.updater", "com.miui.greenguard", "com.xiaomi.aiasst.service", 
        "com.xiaomi.aiasst.vision", "org.mipay.android.manager", "com.xiaomi.mirror", "com.miui.tsmclient", 
        "com.miui.voicetrigger", "com.xiaomi.xaee", "com.xiaomi.aireco", "com.unionpay.tsmservice.mi", 
        "com.miui.vipservice", "com.fido.asm", "com.miui.carlink", "com.xiaomi.joyose", "com.miui.calculator", 
        "com.sohu.inputmethod.sogou.xiaomi", "com.miui.cleaner", "com.bsp.catchlog"
    };
    package_count = sizeof(default_debloat_list) / sizeof(default_debloat_list[0]);

    for (int i = 0; i < package_count; i++) {
        strncpy(debloat_list[i], default_debloat_list[i], BUFFER_SIZE);
    }

    system("cls");
    const char *ascii_art[] = {
        " __  __                                   ",
        " |   |    __.  .___  ` ____    __.  , __  ",
        " |___|  .'   \\ /   \\ |    /  .'   \\ |'  `.",
        " |   |  |    | |   ' |  ,/   |    | |    |",
        " /   /   `._.' /     / /__.'  `._.' /    |",
        "                       `                   "
    };

    int lines_count = sizeof(ascii_art) / sizeof(ascii_art[0]);
    printf("\033[0;36m");
    animate_ascii_art(ascii_art, lines_count);
    print_centered("\t Please connect your Xiaomi device\033[0;36m");

    adb_executable("wait-for-device", NULL);

    char serial[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    get_device_serial(serial, sizeof(serial));
    snprintf(message, sizeof(message), " %s %s Detected...", get_system_property("ro.product.brand"), serial);
    print_centered(message);

    if (strcmp(get_system_property("ro.product.brand"), "Xiaomi") != 0) {
        print_centered("\e[0;31m\t       This device is not a valid Xiaomi Device.\e[0;37m\n");
        exit(EXIT_FAILURE);
    } else if (adb_exe == NULL) {
        print_centered("\e[0;31m\t       Can't find the adb tools from the current directory, swapping to the default path.\e[0;37m\n");
        print_centered("\e[0;31m\t       if it doesn't work please don't report this as an issue!\e[0;37m\n");
    }

    for (int i = 0; i < package_count; i++) {
        uninstall_package(debloat_list[i]);
    }

    printf("\n");
    print_centered("Adding final touches, please wait..");
    adb_executable("wait-for-device", NULL);
    adb_executable("shell", "pm compile -a -f --check-prof false -m everything");
    adb_executable("shell", "pm compile -a -f --check-prof false --compile-layouts");
    adb_executable("shell", "pm bg-dexopt-job");
    adb_executable("reboot", NULL);
    
    return 0;
}
