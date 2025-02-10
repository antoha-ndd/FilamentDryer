#include <Arduino.h>
#include <Objects.h>
#include <dht22.h>
#include <ObjectTimer.h>
#include <ssd1306.h>
#include <encoder.h>
#include <varbasetypes.h>

#include <EEPROM.h>

void Timer1_OnTimeout(TTimer *Timer);
void EncoderPress(TEncoder *Encoder);
void OnLeft(TEncoder *Encoder);
void OnRight(TEncoder *Encoder);

int StateT = 0;

typedef struct Settings
{
  int Temp;
} TSettings;

class TApp : public TApplication
{
public:
  Tdht22 *dht;
  TTimer *Timer1;
  Tssd1306 *Screen;
  TEncoder *Encoder;
  TBool *EditMode, *HeaterOn;
  TSettings Settings;

  TApp() : TApplication()
  {

    EEPROM.get(0, Settings);

    if (Settings.Temp < 1 || Settings.Temp > 60)
      Settings.Temp = 1;

    dht = new Tdht22(7);
    delay(1000);
    dht->Register(this);

    Screen = new Tssd1306();
    Screen->setTextSize(2);
    Screen->println("START");
    Screen->display();

    Timer1 = new TTimer();
    Timer1->Register(this);
    Timer1->OnTimeout = Timer1_OnTimeout;
    Timer1->Start(500);

    Encoder = new TEncoder(5, 3, 4);
    Encoder->OnPress = EncoderPress;
    Encoder->OnRight = OnRight;
    Encoder->OnLeft = OnLeft;
    Encoder->setDirection(false);
    Encoder->tick();
    Encoder->resetStates();
    Encoder->Register(this);

    EditMode = new TBool();
    HeaterOn = new TBool();
    EditMode->SetValue(false);
    HeaterOn->SetValue(false);

    pinMode(9, OUTPUT);
    digitalWrite(9, 1);

    // Serial.println("DEST TEMP : " + String(Settings.Temp));
    // Serial.println("DEST HUM : " + String(Settings.Hum));
  };
};

TApp *App;

void DrawMenu()
{

  String Lines[5] = {">---", "+>--", "-+>-", "--+>", "---+"};
  static byte Mark = 0;

  if (!App->EditMode->GetValue())
  {
    App->Screen->clearDisplay();
    App->Screen->setTextSize(2);
    App->Screen->setCursor(10, 0);
    App->Screen->print(App->dht->Temperature(), 1);
    App->Screen->print("c");

    App->Screen->setCursor(10, 16);
    App->Screen->print(App->dht->Humidity(), 1);
    App->Screen->print("%");

    if (App->HeaterOn->GetValue())
    {
      App->Screen->setTextSize(1);
      App->Screen->setCursor(84, 4);
      App->Screen->print(Lines[Mark]);
      App->Screen->setCursor(89, 14);
      App->Screen->print("ON");
    }

    if (StateT < 0)
    {
      App->Screen->setTextSize(1);
      App->Screen->setCursor(0, 5);
      App->Screen->print("!");
    }

    App->Screen->display();

    Mark++;
    if (Mark == 5)
      Mark = 0;
  }
  else
  {

    App->Screen->clearDisplay();
    App->Screen->setTextSize(2);
    App->Screen->setCursor(10, 0);
    App->Screen->print("T : ");
    App->Screen->print( String(App->Settings.Temp));
    App->Screen->print("c");
    App->Screen->display();
  }
}

void EncoderPress(TEncoder *Encoder)
{

  App->EditMode->SetValue(!App->EditMode->GetValue());
  EEPROM.put(0, App->Settings);

  DrawMenu();
}

void OnLeft(TEncoder *Encoder)
{

  if (App->EditMode->GetValue())
  {

    if (App->Settings.Temp > 1)
      App->Settings.Temp--;

    DrawMenu();
  }
};

void OnRight(TEncoder *Encoder)
{
  if (App->EditMode->GetValue())
  {
    if (App->Settings.Temp < 65)
      App->Settings.Temp++;

    DrawMenu();
  }
};

void CheckHeat()
{

  float T = App->dht->Temperature(), Ts = App->Settings.Temp;

  if (T > Ts)
    StateT = -1;
  if (T < (Ts - 1))
    StateT = 1;

  if (StateT > 0)
  {
    App->HeaterOn->SetValue(true);
    digitalWrite(9, false);
  }
  else if (StateT < 0)
  {
    App->HeaterOn->SetValue(false);
    digitalWrite(9, true);
  }
}

void Timer1_OnTimeout(TTimer *Timer)
{

  App->dht->UpdateValues();
  CheckHeat();
  DrawMenu();
};

unsigned long int TimerSource()
{
  return millis();
};

void setup()
{

  GetTimerValue = TimerSource;
  //  Serial.begin(9600);

  App = new TApp();
  App->Run();

  // Serial.println("App started!");
}

void loop()
{

  App->Idle();
}
