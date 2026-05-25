#include "AlarmClock.h"
#include "MyOS.h"
void AlarmClock::Begin()
{
    AskForTime();
}

void AlarmClock::AskForTime()
{
    int CurrentHour;  // = mainOS->AskFromUserForString("What the Current Hour (2 Digits)", false, false, true).toInt();
    int CurrentMinut; // = mainOS->AskFromUserForString("What the Current Minut (2 Digits)", false, false, true).toInt();

    int WakeUpHour;  // = mainOS->AskFromUserForString("Wake Up Hour (2 Digits)", false, false, true).toInt();
    int WakeUpMinut; // = mainOS->AskFromUserForString("Wake Up Minut (2 Digits)", false, false, true).toInt();

    bool DoneInput = false;
    while (!DoneInput)
    {

        CurrentHour = mainOS->AskFromUserForString("What the Current Hour (0-23, 13 is 1 PM)", false, false, true).toInt();
        CurrentMinut = mainOS->AskFromUserForString("What the Current Minut (0-59)", false, false, true).toInt();

        WakeUpHour = mainOS->AskFromUserForString("Wake Up Hour (0-23, 13 is 1 PM)", false, false, true).toInt();
        WakeUpMinut = mainOS->AskFromUserForString("Wake Up Minut (0-59)", false, false, true).toInt();

        // cheack if all nombers are correct
        if (CurrentHour >= 0 && CurrentHour <= 23 && CurrentMinut >= 0 && CurrentMinut <= 59 && WakeUpHour >= 0 && WakeUpHour <= 23 && WakeUpMinut >= 0 && WakeUpMinut <= 59)
        {
            DoneInput = true;
        }
        else
        {
            mainOS->ShowOnScreenMessege("digits not valid", 1000);
        }
    }

    /*     int CurrentHour = mainOS->AskFromUserForString("What the Current Hour (2 Digits)", false, false, true).toInt();
        int CurrentMinut = mainOS->AskFromUserForString("What the Current Minut (2 Digits)", false, false, true).toInt();

        int WakeUpHour = mainOS->AskFromUserForString("Wake Up Hour (2 Digits)", false, false, true).toInt();
        int WakeUpMinut = mainOS->AskFromUserForString("Wake Up Minut (2 Digits)", false, false, true).toInt();
     */

    int currentTotalMinutes = (CurrentHour * 60) + CurrentMinut;
    int wakeUpTotalMinutes = (WakeUpHour * 60) + WakeUpMinut;

    int diffMinutes;
    mainOS->SetFastBootVar(8);
    mainOS->ShowOnScreenMessege(String("Alarm Clock Set!! The Cardputer Will Go To Deep Sleep Mode To Save Battery Life, Don't turn off the power switch To anable the alarm to work!"));
    //String msg2="";
    

    if (wakeUpTotalMinutes > currentTotalMinutes)
    {
        // זמן היקיצה היום (למשל: עכשיו 10:00, יקיצה ב-12:00)
        diffMinutes = wakeUpTotalMinutes - currentTotalMinutes;
    }
    else
    {
        // זמן היקיצה הוא מחר (למשל: עכשיו 22:00, יקיצה ב-07:00)
        diffMinutes = (1440 - currentTotalMinutes) + wakeUpTotalMinutes;
        // 1440 זה מספר הדקות ביממה (24*60)
    }

    // המרה למיקרו-שניות
    uint64_t sleepTimeMicroseconds = (uint64_t)diffMinutes * 60 * 1000000;
    mainOS->ShowOnScreenMessege(String("Deep Sleep for " + String(diffMinutes) + " Minutes"));

    /* Serial.print("Sleeping for ");
    Serial.print(diffMinutes);
    Serial.println(" minutes."); */

    // הגדרת הטיימר להתעוררות
    esp_sleep_enable_timer_wakeup(sleepTimeMicroseconds);

    // כניסה לשינה עמוקה
    esp_deep_sleep_start();
}

void AlarmClock::Loop()
{
}

void AlarmClock::Draw()
{
}
