#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifdef HAVE_EMBEDDED_DATA
#include "embedded_data.h"
#endif

#define TOOL_VERSION "1.0.0"

void print_banner() {
    printf("====================================================================\n");
    printf("     BlackBerry Passport (SQW100-1) VoLTE Revival Tool v%s\n", TOOL_VERSION);
    printf("     Automatic 4G LTE VoLTE Patcher & Deployer (Viettel / IR.92)\n");
    printf("====================================================================\n\n");
}

int write_embedded_file(const char *path, const unsigned char *data, unsigned int len) {
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
    char passport_ip[128] = "192.168.0.101";
    char user_input[128] = "";
    char temp_dir[MAX_PATH];
    char work_dir[MAX_PATH];
    char key_path[MAX_PATH];
    char payload_path[MAX_PATH];
    char ssh_bin[MAX_PATH] = "ssh";
    char scp_bin[MAX_PATH] = "scp";
    char cmd[4096];

    print_banner();

    // Determine OpenSSH path
    if (file_exists("C:\\Windows\\System32\\OpenSSH\\ssh.exe")) {
        strcpy(ssh_bin, "\"C:\\Windows\\System32\\OpenSSH\\ssh.exe\"");
        strcpy(scp_bin, "\"C:\\Windows\\System32\\OpenSSH\\scp.exe\"");
    } else {
        strcpy(ssh_bin, "ssh");
        strcpy(scp_bin, "scp");
    }

    // Get Passport IP
    if (argc > 1) {
        strncpy(passport_ip, argv[1], sizeof(passport_ip) - 1);
        passport_ip[sizeof(passport_ip) - 1] = '\0';
    } else {
        printf("Enter BlackBerry Passport IP address [%s]: ", passport_ip);
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

    printf("\n[*] Target Passport IP: %s\n", passport_ip);

    // Setup temporary working directory
    if (!GetTempPathA(sizeof(temp_dir), temp_dir)) {
        strcpy(temp_dir, "C:\\Temp");
    }
    snprintf(work_dir, sizeof(work_dir), "%sbb10_volte_installer", temp_dir);
    CreateDirectoryA(work_dir, NULL);

    snprintf(key_path, sizeof(key_path), "%s\\id_rsa", work_dir);
    snprintf(payload_path, sizeof(payload_path), "%s\\payload.tar.gz", work_dir);

#ifdef HAVE_EMBEDDED_DATA
    printf("[*] Extracting embedded private key and VoLTE patch payload...\n");
    if (!write_embedded_file(key_path, EMBEDDED_SSH_KEY, EMBEDDED_SSH_KEY_len)) {
        printf("[-] Failed to write SSH key to temporary directory.\n");
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }
    if (!write_embedded_file(payload_path, EMBEDDED_PAYLOAD, EMBEDDED_PAYLOAD_len)) {
        printf("[-] Failed to write payload to temporary directory.\n");
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }
    printf("[+] Extracted %u bytes (Key) and %u bytes (VoLTE Bundle).\n", 
           EMBEDDED_SSH_KEY_len, EMBEDDED_PAYLOAD_len);
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
             "-o StrictHostKeyChecking=no -o UserKnownHostsFile=NUL "
             "-o KexAlgorithms=+diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha256 "
             "-o Ciphers=+aes128-cbc,3des-cbc,aes256-cbc,aes128-ctr,aes256-ctr "
             "-o HostKeyAlgorithms=+ssh-rsa "
             "-o ConnectTimeout=8 "
             "root@%s \"uname -a\"",
             ssh_bin, key_path, passport_ip);

    int ret = system(cmd);
    if (ret != 0) {
        printf("\n[-] Error: Could not connect to Passport at %s.\n", passport_ip);
        printf("    Please ensure:\n");
        printf("    1. The Passport is on the same Wi-Fi network or USB IP.\n");
        printf("    2. Development Mode / SSH is enabled on port 22 with root access.\n");
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }

    // Upload payload
    printf("\n[*] Uploading VoLTE package to Passport (/tmp/payload.tar.gz)...\n");
    snprintf(cmd, sizeof(cmd),
             "%s -O -P 22 -i \"%s\" "
             "-o StrictHostKeyChecking=no -o UserKnownHostsFile=NUL "
             "-o KexAlgorithms=+diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha256 "
             "-o Ciphers=+aes128-cbc,3des-cbc,aes256-cbc,aes128-ctr,aes256-ctr "
             "-o HostKeyAlgorithms=+ssh-rsa "
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
    printf("\n[*] Deploying and launching VoLTE service on Passport...\n");
    snprintf(cmd, sizeof(cmd),
             "%s -p 22 -i \"%s\" "
             "-o StrictHostKeyChecking=no -o UserKnownHostsFile=NUL "
             "-o KexAlgorithms=+diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha256 "
             "-o Ciphers=+aes128-cbc,3des-cbc,aes256-cbc,aes128-ctr,aes256-ctr "
             "-o HostKeyAlgorithms=+ssh-rsa "
             "root@%s \""
             "cd /tmp && rm -rf passport_volte_dist && tar -xzf payload.tar.gz && /bin/sh /tmp/passport_volte_dist/deploy_device.sh"
             "\"",
             ssh_bin, key_path, passport_ip);

    ret = system(cmd);

    // Cleanup local temp
    DeleteFileA(key_path);
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
