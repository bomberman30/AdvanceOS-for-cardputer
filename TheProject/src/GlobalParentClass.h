#pragma once

class MyOS; // forward declaration

class GlobalParentClass
{
public:
    MyOS *mainOS; //may be to delete in future . we alredy have inline variable in MyOS.h
    bool showTopBar = true;
    GlobalParentClass(MyOS *os) : mainOS(os) {}
    virtual ~GlobalParentClass() {}
    int TopOffset = 0;
    virtual void Begin();
    virtual void Loop();
    virtual void Draw();
    virtual void OnExit();

    void BackToMainMenu();
    bool AddWillBeDeleted=false;
private:
};
