#include "GlobalParentClass.h"
#include <M5Cardputer.h>
#include "./MyOS.h"
#include "Classes/MainMenuV2.h"
void GlobalParentClass::Begin()
{
}
void GlobalParentClass::Loop()
{
    if (mainOS->NewKey.ifKeyJustPress('`')) 
    {
        BackToMainMenu();
    }
}
void GlobalParentClass::OnExit()
{
}
void GlobalParentClass::BackToMainMenu()
{
    mainOS->ChangeMenu(new MainMenuV2(mainOS));
    return;
}