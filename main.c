#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifdef HAVE_EMBEDDED_DATA
#include "embedded_data.h"
#endif

#define TOOL_VERSION "1.1.0"

void print_banner() {
    printf("====================================================================\n");
    printf("     BlackBerry 10 (Passport / Q10 / Q20) VoLTE Revival Tool v%s\n", TOOL_VERSION);
    printf("     Automatic 4G LTE VoLTE Patcher & Deployer (Vietnam Carriers)\n");
    printf("====================================================================\n\n");
}

int write_embedded_file(const char *path, const unsigned char *data, unsigned int len) {
    SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);
    DeleteFileA(path);
    FILE *f = fopen(path, "wb");
    if (!f) {
        printf("[-] Error: Unable to create file '%s'\n", path);
        return 0;
    }
    size_t written = fwrite(data, 1, len, f);
    fclose(f);
    return written == len;
}

int file_exists(const char *path) {
    DWORD dwAttrib = GetFileAttributesA(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int main(int argc, char *argv[]) {
    char passport_ip[128] = "169.254.0.1";
    char user_input[128] = "";
    char temp_dir[MAX_PATH];
    char work_dir[MAX_PATH];
    char key_path[MAX_PATH];
    char payload_path[MAX_PATH];
    char ssh_bin[MAX_PATH] = "ssh";
    char scp_bin[MAX_PATH] = "scp";
    char cmd[4096];

    print_banner();

    // Determine OpenSSH path (no quotes around binary path to prevent cmd.exe quote-stripping)
    if (file_exists("C:\\Windows\\System32\\OpenSSH\\ssh.exe")) {
        strcpy(ssh_bin, "C:\\Windows\\System32\\OpenSSH\\ssh.exe");
        strcpy(scp_bin, "C:\\Windows\\System32\\OpenSSH\\scp.exe");
    } else {
        strcpy(ssh_bin, "ssh");
        strcpy(scp_bin, "scp");
    }

    char carrier_choice[32] = "auto";

    // Get Device IP
    if (argc > 1) {
        strncpy(passport_ip, argv[1], sizeof(passport_ip) - 1);
        passport_ip[sizeof(passport_ip) - 1] = '\0';
    } else {
        printf("Enter BlackBerry 10 device IP address [%s]: ", passport_ip);
        if (fgets(user_input, sizeof(user_input), stdin)) {
            char *p = user_input;
            while (*p) {
                if (*p == '\r' || *p == '\n') {
                    *p = '\0';
                    break;
                }
                p++;
            }
            if (strlen(user_input) > 0) {
                strncpy(passport_ip, user_input, sizeof(passport_ip) - 1);
                passport_ip[sizeof(passport_ip) - 1] = '\0';
            }
        }
    }

    // Get Carrier Selection
    if (argc > 2) {
        strncpy(carrier_choice, argv[2], sizeof(carrier_choice) - 1);
        carrier_choice[sizeof(carrier_choice) - 1] = '\0';
    } else {
        printf("\nSelect Carrier Profile:\n");
        printf("  1. Auto-detect from SIM (Recommended)\n");
        printf("  2. Viettel (452-04)\n");
        printf("  3. MobiFone (452-01)\n");
        printf("  4. VinaPhone (452-02)\n");
        printf("  5. Universal Vietnam (All 452 carriers)\n");
        printf("Enter choice [1]: ");
        if (fgets(user_input, sizeof(user_input), stdin)) {
            char *p = user_input;
            while (*p) {
                if (*p == '\r' || *p == '\n') {
                    *p = '\0';
                    break;
                }
                p++;
            }
            if (strcmp(user_input, "2") == 0 || strcasecmp(user_input, "viettel") == 0) {
                strcpy(carrier_choice, "viettel");
            } else if (strcmp(user_input, "3") == 0 || strcasecmp(user_input, "mobifone") == 0) {
                strcpy(carrier_choice, "mobifone");
            } else if (strcmp(user_input, "4") == 0 || strcasecmp(user_input, "vinaphone") == 0) {
                strcpy(carrier_choice, "vinaphone");
            } else if (strcmp(user_input, "5") == 0 || strcasecmp(user_input, "all") == 0) {
                strcpy(carrier_choice, "vietnam-all");
            } else {
                strcpy(carrier_choice, "auto");
            }
        }
    }

    printf("\n[*] Target Device IP: %s\n", passport_ip);
    printf("[*] Carrier Profile : %s\n", carrier_choice);

    // Setup temporary working directory
    if (!GetTempPathA(sizeof(temp_dir), temp_dir)) {
        strcpy(temp_dir, "C:\\Temp");
    }
    // Ensure trailing slash
    size_t tlen = strlen(temp_dir);
    if (tlen > 0 && temp_dir[tlen - 1] != '\\' && temp_dir[tlen - 1] != '/') {
        strcat(temp_dir, "\\");
    }

    // Clean up any legacy temp folder from older v1.0.0 versions
    char legacy_dir[MAX_PATH], legacy_file[MAX_PATH];
    snprintf(legacy_dir, sizeof(legacy_dir), "%sbb10_volte_installer", temp_dir);
    snprintf(legacy_file, sizeof(legacy_file), "%s\\id_rsa", legacy_dir);
    SetFileAttributesA(legacy_file, FILE_ATTRIBUTE_NORMAL);
    DeleteFileA(legacy_file);
    snprintf(legacy_file, sizeof(legacy_file), "%s\\payload.tar.gz", legacy_dir);
    SetFileAttributesA(legacy_file, FILE_ATTRIBUTE_NORMAL);
    DeleteFileA(legacy_file);
    RemoveDirectoryA(legacy_dir);

    // Use a unique per-process temp folder to prevent ACL or concurrency collisions
    snprintf(work_dir, sizeof(work_dir), "%sbb10_volte_%lu_%lu", 
             temp_dir, (unsigned long)GetCurrentProcessId(), (unsigned long)GetTickCount());
    if (!CreateDirectoryA(work_dir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        // Fallback if temp directory creation failed
        snprintf(work_dir, sizeof(work_dir), ".\\bb10_volte_temp");
        CreateDirectoryA(work_dir, NULL);
    }

    snprintf(key_path, sizeof(key_path), "%s\\id_rsa", work_dir);
    snprintf(payload_path, sizeof(payload_path), "%s\\payload.tar.gz", work_dir);

    int custom_key = 0;
    if (file_exists("id_rsa")) {
        printf("[*] Found custom 'id_rsa' in current folder, using it...\n");
        CopyFileA("id_rsa", key_path, FALSE);
        custom_key = 1;
    }

#ifdef HAVE_EMBEDDED_DATA
    if (!custom_key) {
        printf("[*] Extracting embedded private key and VoLTE patch payload...\n");
        if (!write_embedded_file(key_path, EMBEDDED_SSH_KEY, EMBEDDED_SSH_KEY_len)) {
            printf("[-] Failed to write SSH key to temporary directory.\n");
            printf("\nPress Enter to exit...");
            getchar();
            return 1;
        }
    } else {
        printf("[*] Extracting VoLTE patch payload...\n");
    }
    if (!write_embedded_file(payload_path, EMBEDDED_PAYLOAD, EMBEDDED_PAYLOAD_len)) {
        printf("[-] Failed to write payload to temporary directory.\n");
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }
    printf("[+] Ready: Key and VoLTE Bundle staged successfully.\n");
#else
    // Standalone mode: load key and payload from current directory or specific paths
    if (file_exists("id_rsa")) {
        CopyFileA("id_rsa", key_path, FALSE);
    } else {
        printf("[-] Error: 'id_rsa' not found in current folder. Please place your device root SSH key here.\n");
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }

    if (file_exists("passport_volte_package.tar.gz")) {
        CopyFileA("passport_volte_package.tar.gz", payload_path, FALSE);
    } else if (file_exists("payload.tar.gz")) {
        CopyFileA("payload.tar.gz", payload_path, FALSE);
    } else {
        printf("[-] Error: 'passport_volte_package.tar.gz' or 'payload.tar.gz' not found in current folder.\n");
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }
    printf("[+] Loaded SSH key and payload bundle from current directory.\n");
#endif

    // Secure private key permissions on Windows to satisfy OpenSSH client
    snprintf(cmd, sizeof(cmd), "icacls \"%s\" /inheritance:r /grant:r \"%%USERNAME%%:R\" >NUL 2>&1", key_path);
    system(cmd);

    // Test connectivity
    printf("\n[*] Connecting to root@%s (testing SSH)...\n", passport_ip);
    snprintf(cmd, sizeof(cmd),
             "%s -p 22 -i \"%s\" "
             "-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null "
             "-o KexAlgorithms=+diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha256 "
             "-o Ciphers=+aes128-ctr,aes192-ctr,aes256-ctr,aes128-cbc,3des-cbc,aes256-cbc "
             "-o HostKeyAlgorithms=+ssh-rsa "
             "-o PubkeyAcceptedKeyTypes=+ssh-rsa "
             "-o MACs=+hmac-sha1 "
             "-o ConnectTimeout=12 "
             "root@%s \"uname -a\"",
             ssh_bin, key_path, passport_ip);

    int ret = system(cmd);
    if (ret != 0) {
        printf("\n[-] Error: Could not connect to device at %s (exit code %d).\n", passport_ip, ret);
        printf("    Diagnosis & Troubleshooting:\n");
        printf("    1. IP Address: On phone, go to Settings -> Security & Privacy -> Development Mode.\n");
        printf("       Check the exact 'Development IPv4 Address' shown on your phone's screen.\n");
        printf("    2. If using USB: In Settings -> Storage & Access, ensure USB is set to 'Connect to Windows'.\n");
        printf("    3. If using Wi-Fi: Ensure phone and PC are connected to the SAME Wi-Fi network.\n");
        printf("    4. SSH Key: If you used a personal AutoRoot key, put your 'id_rsa' in this tool's folder.\n");
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }

    // Upload payload
    printf("\n[*] Uploading VoLTE package to device (/tmp/payload.tar.gz)...\n");
    snprintf(cmd, sizeof(cmd),
             "%s -O -P 22 -i \"%s\" "
             "-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null "
             "-o KexAlgorithms=+diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha256 "
             "-o Ciphers=+aes128-ctr,aes192-ctr,aes256-ctr,aes128-cbc,3des-cbc,aes256-cbc "
             "-o HostKeyAlgorithms=+ssh-rsa "
             "-o PubkeyAcceptedKeyTypes=+ssh-rsa "
             "-o MACs=+hmac-sha1 "
             "\"%s\" root@%s:/tmp/payload.tar.gz",
             scp_bin, key_path, payload_path, passport_ip);

    ret = system(cmd);
    if (ret != 0) {
        printf("[-] SCP upload failed with exit code %d.\n", ret);
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }
    printf("[+] VoLTE bundle uploaded successfully.\n");

    // Execute remote installer
    printf("\n[*] Deploying and launching VoLTE service on device...\n");
    snprintf(cmd, sizeof(cmd),
             "%s -p 22 -i \"%s\" "
             "-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null "
             "-o KexAlgorithms=+diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha256 "
             "-o Ciphers=+aes128-ctr,aes192-ctr,aes256-ctr,aes128-cbc,3des-cbc,aes256-cbc "
             "-o HostKeyAlgorithms=+ssh-rsa "
             "-o PubkeyAcceptedKeyTypes=+ssh-rsa "
             "-o MACs=+hmac-sha1 "
             "root@%s \""
             "cd /tmp && rm -rf passport_volte_dist && mkdir -p passport_volte_dist && tar -xzf payload.tar.gz -C passport_volte_dist && /bin/sh /tmp/passport_volte_dist/deploy_device.sh %s"
             "\"",
             ssh_bin, key_path, passport_ip, carrier_choice);

    ret = system(cmd);

    // Cleanup local temp
    SetFileAttributesA(key_path, FILE_ATTRIBUTE_NORMAL);
    DeleteFileA(key_path);
    SetFileAttributesA(payload_path, FILE_ATTRIBUTE_NORMAL);
    DeleteFileA(payload_path);
    RemoveDirectoryA(work_dir);

    printf("\n====================================================================\n");
    if (ret == 0) {
        printf("[+] VoLTE Revival deployment process completed!\n");
        printf("    Check your BlackBerry Passport screen:\n");
        printf("    - LTE network active\n");
        printf("    - Ready for incoming & outgoing VoLTE calling\n");
    } else {
        printf("[-] Deployment completed with warnings or status code %d.\n", ret);
    }
    printf("====================================================================\n");

    printf("\nPress Enter to exit...");
    getchar();
    return 0;
}
