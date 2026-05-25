#include "PasswordVault.h"
#include "MyOS.h"
#include <ArduinoJson.h>
#include <FS.h>
#include <SD.h>
#include "mbedtls/aes.h"
#include "./Classes/MainMenuV2.h"
#include <mbedtls/md.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/gcm.h>
#include <esp_random.h>

#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define PBKDF2_ITERATIONS 10000






// Path and key settings
const char *VAULT_PATH = "/AdvanceOS/Vault/data.bin";

void PasswordVault::Begin()
{
    if (!SD.exists("/AdvanceOS/Vault"))
    {
        SD.mkdir("/AdvanceOS/Vault");
    }

    if (SD.exists(VAULT_PATH))
    {
        String pass = mainOS->AskFromUserForString("Enter Vault Password",false,true);

        if (!LoadVault(pass))
        {
            mainOS->ShowOnScreenMessege("Wrong Password", 1000);
            mainOS->ChangeMenu(new MainMenuV2(mainOS));
            return;
        }

        Password = pass;
    }
    else
    {
        Password = mainOS->AskFromUserForString("Create Vault Password",false);

        doc.to<JsonArray>(); // Create an empty array
        SaveVault(Password);
    }
}

// Helper function for AES encryption/decryption
void PasswordVault::encryptDecrypt(uint8_t *input, size_t inputLen, uint8_t *output, String key, bool encrypt)
{
    mbedtls_aes_context aes;
    uint8_t key256[32] = {0};
    // Generate a 32-byte key from the user's password
    memcpy(key256, key.c_str(), min((size_t)key.length(), (size_t)32));

    mbedtls_aes_init(&aes);
    if (encrypt)
    {
        mbedtls_aes_setkey_enc(&aes, key256, 256);
    }
    else
    {
        mbedtls_aes_setkey_dec(&aes, key256, 256);
    }

    // Encrypt in 16-byte blocks (ECB for simplicity, or CBC for higher security)
    for (size_t i = 0; i < inputLen; i += 16)
    {
        mbedtls_aes_crypt_ecb(&aes, encrypt ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT, input + i, output + i);
    }
    mbedtls_aes_free(&aes);
}

void PasswordVault::SaveVault(String userPassword) {
    String jsonString;
    serializeJson(doc, jsonString);
    size_t originalLen = jsonString.length();

    uint8_t salt[SALT_SIZE];
    uint8_t iv[IV_SIZE];
    uint8_t tag[TAG_SIZE];
    uint8_t derivedKey[32];
    
    esp_fill_random(salt, SALT_SIZE);
    esp_fill_random(iv, IV_SIZE);

    mbedtls_md_context_t sha_ctx;
    mbedtls_md_init(&sha_ctx);
    mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    
    mbedtls_pkcs5_pbkdf2_hmac(&sha_ctx, 
                              (const unsigned char*)userPassword.c_str(), userPassword.length(),
                              salt, SALT_SIZE, PBKDF2_ITERATIONS, 32, derivedKey);
    mbedtls_md_free(&sha_ctx);
    // ---------------------------------------

    uint8_t *output = (uint8_t *)malloc(originalLen);
    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, derivedKey, 256);
    
    mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, originalLen, 
                              iv, IV_SIZE, NULL, 0, 
                              (const uint8_t*)jsonString.c_str(), output, 
                              TAG_SIZE, tag);

    SD.remove(VAULT_PATH);
    File file = SD.open(VAULT_PATH, FILE_WRITE);
    file.write(salt, SALT_SIZE);
    file.write(iv, IV_SIZE);
    file.write(tag, TAG_SIZE);
    file.write((uint8_t *)&originalLen, sizeof(size_t));
    file.write(output, originalLen);
    file.close();

    mbedtls_gcm_free(&gcm);
    free(output);
}

bool PasswordVault::LoadVault(String userPassword) {
    if (!SD.exists(VAULT_PATH)) return false;

    File file = SD.open(VAULT_PATH, FILE_READ);
    uint8_t salt[SALT_SIZE];
    uint8_t iv[IV_SIZE];
    uint8_t tag[TAG_SIZE];
    size_t originalLen;

    file.read(salt, SALT_SIZE);
    file.read(iv, IV_SIZE);
    file.read(tag, TAG_SIZE);
    file.read((uint8_t *)&originalLen, sizeof(size_t));

    uint8_t *input = (uint8_t *)malloc(originalLen);
    uint8_t *output = (uint8_t *)malloc(originalLen + 1);
    file.read(input, originalLen);
    file.close();

    uint8_t derivedKey[32]; 
    mbedtls_md_context_t sha_ctx;
    mbedtls_md_init(&sha_ctx);
    mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    
    mbedtls_pkcs5_pbkdf2_hmac(&sha_ctx, 
                              (const unsigned char*)userPassword.c_str(), userPassword.length(),
                              salt, SALT_SIZE, PBKDF2_ITERATIONS, 32, derivedKey);
    mbedtls_md_free(&sha_ctx);
    // ----------------------------

    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, derivedKey, 256);
    
    int status = mbedtls_gcm_auth_decrypt(&gcm, originalLen, iv, IV_SIZE, 
                                          NULL, 0, tag, TAG_SIZE, 
                                          input, output);
    
    bool success = false;
    if (status == 0) {
        output[originalLen] = '\0';
        doc.clear();
        DeserializationError error = deserializeJson(doc, (char *)output);
        success = (error == DeserializationError::Ok);
    }

    mbedtls_gcm_free(&gcm);
    free(input);
    free(output);
    return success;
}


void PasswordVault::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`'))
    {
        mainOS->ChangeMenu(new MainMenuV2(mainOS));
    }
    size_t itemCount = doc.as<JsonArray>().size();

    if (mainOS->NewKey.ifKeyJustPress(';')) 
    {
        if (selectedIndex > 0)
            selectedIndex--;
    }
    if (mainOS->NewKey.ifKeyJustPress('='))
    {
        CreateEntry();
    }
    if (mainOS->NewKey.ifKeyJustPress(KEY_BACKSPACE))
    {
        if (mainOS->AskSomthing("Are You sure You Want To Delete Entry?"))
        {DeleteEntry();}
    }
    if (mainOS->NewKey.ifKeyJustPress('.')) 
    {
        if (selectedIndex < itemCount - 1)
            selectedIndex++;
    }

    if (!mainOS->screenOff)
    {
        Draw();
    }
}

void PasswordVault::Draw()
{
    // Create Sprite
    mainOS->sprite.createSprite(SCREEN_W, SCREEN_H - TopOffset);
    mainOS->sprite.clear(BLACK);
    mainOS->sprite.setTextSize(1);

    JsonArray arr = doc.as<JsonArray>();
    int yPos = 5;
    int lineHeight = 55; // Increased height to accommodate 3 lines + gaps

    // Scrolling calculation: how many items fit on the screen
    int maxVisibleItems = (SCREEN_H - TopOffset) / lineHeight;
    int scrollOffset = 0;
    if (selectedIndex >= maxVisibleItems)
    {
        scrollOffset = selectedIndex - maxVisibleItems + 1;
    }

    if (arr.size() == 0)
    {
        mainOS->sprite.setTextColor(DARKGREY);
        mainOS->sprite.setCursor(10, 10);
        mainOS->sprite.print("No Passwords Found. Press '+' to Add.");
    }
    else
    {
        for (int i = scrollOffset; i < arr.size(); i++)
        {
            // If out of screen bounds, break
            if (yPos + lineHeight > (SCREEN_H - TopOffset))
                break;

            // Get data from JSON
            const char *title = arr[i]["t"] | "No Title";
            const char *user = arr[i]["u"] | "No User";
            const char *pass = arr[i]["p"] | "No Pass";

            // Draw blue background for selected item
            if (i == selectedIndex)
            {
                mainOS->sprite.fillRect(0, yPos - 2, SCREEN_W, lineHeight - 4, BLUE);
                mainOS->sprite.setTextColor(WHITE);
            }
            else
            {
                mainOS->sprite.setTextColor(GREEN);
            }

            // Line 1: Title
            mainOS->sprite.setCursor(10, yPos);
            mainOS->sprite.print(title);

            // Line 2: Username
            if (i != selectedIndex)
                mainOS->sprite.setTextColor(LIGHTGREY);
            mainOS->sprite.setCursor(10, yPos + 15);
            mainOS->sprite.print("Username: ");
            mainOS->sprite.print(user);

            // Line 3: Password
            if (i != selectedIndex)
                mainOS->sprite.setTextColor(YELLOW);
            mainOS->sprite.setCursor(10, yPos + 30);
            mainOS->sprite.print("Password: ");
            mainOS->sprite.print(pass);

            // Horizontal divider line
            mainOS->sprite.drawFastHLine(0, yPos + lineHeight - 6, SCREEN_W, DARKGREY);

            yPos += lineHeight;
        }
    }

    // Draw scrollbar on the right if there are many items
    if (arr.size() > maxVisibleItems)
    {
        int barHeight = (SCREEN_H - TopOffset) / (arr.size() / (float)maxVisibleItems);
        int barPos = (selectedIndex * (SCREEN_H - TopOffset - barHeight)) / (arr.size() - 1);
        mainOS->sprite.fillRect(SCREEN_W - 4, barPos, 3, barHeight, WHITE);
    }

    mainOS->sprite.pushSprite(0, 0);
    mainOS->sprite.deleteSprite();
}

String PasswordVault::AskForPassword()
{
    Password = mainOS->AskFromUserForString("Set Up Password",false);
    return Password;
}

void PasswordVault::CreateEntry()
{
    String title = mainOS->AskFromUserForString("Title",false);
    String user = mainOS->AskFromUserForString("User",false);
    String pass = mainOS->AskFromUserForString("Password",false);

    JsonArray arr = doc.as<JsonArray>();

    JsonObject obj = arr.add<JsonObject>();

    obj["t"] = title;
    obj["u"] = user;
    obj["p"] = pass;

    SaveVault(Password);
}

void PasswordVault::DeleteEntry()
{
    JsonArray arr = doc.as<JsonArray>();
    
    // Check if there are items and index is in range
    if (arr.size() > 0 && selectedIndex < arr.size()) {
        // Remove item from array
        arr.remove(selectedIndex);
        
        // Update index if we deleted the last one
        if (selectedIndex >= arr.size() && selectedIndex > 0) {
            selectedIndex--;
        }

        // Save back to file to persist changes
        SaveVault(Password);
        
        // Optional: Notify user that deletion succeeded
        mainOS->ShowOnScreenMessege("Deleted!", 500);
    }
}