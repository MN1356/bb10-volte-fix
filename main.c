#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifdef HAVE_EMBEDDED_DATA
#include "embedded_data.h"
#endif

#define TOOL_VERSION "1.2.0"

void print_banner() {
    printf("====================================================================\n");
    printf("     BlackBerry 10 Universal VoLTE Revival Tool v%s\n", TOOL_VERSION);
    printf("     Supported: Passport, Classic Q20, Q10, Z30, Leap, Z10 LTE\n");
    printf("     Carriers: Viettel, MobiFone, VinaPhone, Universal Vietnam\n");
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
    char bootstrap_path[MAX_PATH];
    char ssh_bin[MAX_PATH] = "ssh";
    char scp_bin[MAX_PATH] = "scp";
    char cmd[4096];

    print_banner();

    // Determine OpenSSH path (check local folder first, then Windows OpenSSH, then PATH)
    if (file_exists("ssh.exe") && file_exists("scp.exe")) {
        strcpy(ssh_bin, "ssh.exe");
        strcpy(scp_bin, "scp.exe");
    } else if (file_exists("C:\\Windows\\System32\\OpenSSH\\ssh.exe")) {
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
    snprintf(bootstrap_path, sizeof(bootstrap_path), "%s\\bootstrap.py", work_dir);

    // Staging bootstrap Python helper for clean and reliable remote extraction on QNX
    const char *bootstrap_script =
        "import sys, os, tarfile, shutil\n"
        "p = '/var/rootdata/passport_volte_dist'\n"
        "if os.path.exists(p):\n"
        "    shutil.rmtree(p, ignore_errors=True)\n"
        "try:\n"
        "    os.makedirs(p)\n"
        "except Exception:\n"
        "    pass\n"
        "tar = tarfile.open('/var/rootdata/payload.tar.gz')\n"
        "tar.extractall(p)\n"
        "tar.close()\n"
        "carrier = sys.argv[1] if len(sys.argv) > 1 else 'auto'\n"
        "ret = os.system('/bin/sh ' + p + '/deploy_device.sh ' + carrier)\n"
        "shutil.rmtree(p, ignore_errors=True)\n"
        "if os.path.exists('/var/rootdata/payload.tar.gz'):\n"
        "    try: os.unlink('/var/rootdata/payload.tar.gz')\n"
        "    except Exception: pass\n"
        "if os.path.exists('/var/rootdata/bootstrap.py'):\n"
        "    try: os.unlink('/var/rootdata/bootstrap.py')\n"
        "    except Exception: pass\n"
        "sys.exit(0 if ret == 0 else 1)\n";

    FILE *fbs = fopen(bootstrap_path, "wb");
    if (fbs) {
        fwrite(bootstrap_script, 1, strlen(bootstrap_script), fbs);
        fclose(fbs);
    }

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
             "-o StrictHostKeyChecking=no -o UserKnownHostsFile=NUL "
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
        printf("\n[-] Loi: Khong the ket noi SSH toi root@%s (ma loi %d).\n", passport_ip, ret);
        printf("    ====================================================================\n");
        printf("    NGUYEN NHAN CHINH & HUONG XU LY:\n");
        printf("    1. MAY BLACKBERRY CHUA ROOT BANG AUTOROOT V4:\n");
        printf("       - 'Che do phat trien' (Development Mode) mac dinh chi cho phep\n");
        printf("         tai khoan 'devuser', KHONG cho phep dang nhap quyen 'root'.\n");
        printf("       - May bat buoc phai duoc chay AutoRoot V4 truoc de nap SSH root.\n");
        printf("    2. Kiem tra dia chi IP:\n");
        printf("       - Neu cam cap USB: mac dinh la 169.254.0.1 (Trong Cai dat -> Luu tru\n");
        printf("         & Truy cap: chon 'Ket noi voi Windows').\n");
        printf("       - Neu dung Wi-Fi: vao Cai dat -> Bao mat & rieng tu -> Che do phat\n");
        printf("         trien de xem dung dia chi IP Wi-Fi dang cap cho may BlackBerry.\n");
        printf("    3. Neu may da root bang bo key khac: chep file 'id_rsa' vao cung thu muc tool.\n");
        printf("    ====================================================================\n");
        printf("\nPress Enter to exit...");
        getchar();
        return 1;
    }

    // Upload payload and bootstrap helper
    printf("\n[*] Uploading VoLTE package to device (/var/rootdata/payload.tar.gz)...\n");
    snprintf(cmd, sizeof(cmd),
             "%s -O -P 22 -i \"%s\" "
             "-o StrictHostKeyChecking=no -o UserKnownHostsFile=NUL "
             "-o KexAlgorithms=+diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha256 "
             "-o Ciphers=+aes128-ctr,aes192-ctr,aes256-ctr,aes128-cbc,3des-cbc,aes256-cbc "
             "-o HostKeyAlgorithms=+ssh-rsa "
             "-o PubkeyAcceptedKeyTypes=+ssh-rsa "
             "-o MACs=+hmac-sha1 "
             "\"%s\" \"%s\" root@%s:/var/rootdata/",
             scp_bin, key_path, payload_path, bootstrap_path, passport_ip);

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
             "-o StrictHostKeyChecking=no -o UserKnownHostsFile=NUL "
             "-o KexAlgorithms=+diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha256 "
             "-o Ciphers=+aes128-ctr,aes192-ctr,aes256-ctr,aes128-cbc,3des-cbc,aes256-cbc "
             "-o HostKeyAlgorithms=+ssh-rsa "
             "-o PubkeyAcceptedKeyTypes=+ssh-rsa "
             "-o MACs=+hmac-sha1 "
             "root@%s \"export PYTHONHOME=/usr; export PYTHONPATH=/usr/lib/python32.zip:/usr/lib/python3.2/lib-dynload; /proc/boot/pathtrust '!/var/rootdata/bootstrap.py'; /usr/bin/python3.2 /var/rootdata/bootstrap.py %s\"",
             ssh_bin, key_path, passport_ip, carrier_choice);

    ret = system(cmd);

    // Cleanup local temp
    SetFileAttributesA(key_path, FILE_ATTRIBUTE_NORMAL);
    DeleteFileA(key_path);
    SetFileAttributesA(payload_path, FILE_ATTRIBUTE_NORMAL);
    DeleteFileA(payload_path);
    SetFileAttributesA(bootstrap_path, FILE_ATTRIBUTE_NORMAL);
    DeleteFileA(bootstrap_path);
    RemoveDirectoryA(work_dir);

    printf("\n====================================================================\n");
    if (ret == 0) {
        printf("[+] VoLTE Revival deployment process completed!\n");
        printf("    Check your BlackBerry device screen:\n");
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
