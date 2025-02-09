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

typedef struct Settings
{
  int Hum;
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
  TFloat *DestTemp, *DestHumidity;
  TInt *EditValue;
  TSettings Settings;


  TApp() : TApplication()
  {

    EEPROM.get(0, Settings);

    if (Settings.Temp < 1 || Settings.Temp > 60)
      Settings.Temp = 1;
    if (Settings.Hum < 1 || Settings.Hum > 99)
      Settings.Hum = 1;

    dht = new Tdht22(7);
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

    Encoder->Register(this);

    EditMode = new TBool();
    HeaterOn = new TBool();
    EditMode->SetValue(false);
    HeaterOn->SetValue(false);

    EditValue = new TInt();
    EditValue->SetValue(0);

    DestTemp = new TFloat();
    DestHumidity = new TFloat();
    DestTemp->SetValue(Settings.Temp);
    DestHumidity->SetValue(Settings.Hum);

    pinMode(9,OUTPUT);
    digitalWrite(9,0);

    Serial.println("DEST TEMP : " + String(Settings.Temp));
    Serial.println("DEST HUM : " + String(Settings.Hum));
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
    App->Screen->display();

    Mark++;
    if (Mark == 5)
      Mark = 0;
  }
  else
  {

    if (App->EditValue->GetValue() == 1)
    {

      App->Screen->clearDisplay();
      App->Screen->setTextSize(2);
      App->Screen->setCursor(10, 0);
      App->Screen->print("T : ");
      App->Screen->print(App->DestTemp->GetValue(),0);
      App->Screen->print("c");
      App->Screen->display();
    }
    else if (App->EditValue->GetValue() == 2)
    {

      App->Screen->clearDisplay();
      App->Screen->setTextSize(2);
      App->Screen->setCursor(10, 0);
      App->Screen->print("H : ");
      App->Screen->print(App->DestHumidity->GetValue(),0);
      App->Screen->print("%");
      App->Screen->display();
    }
  }
}

void EncoderPress(TEncoder *Encoder)
{

  if (App->EditMode->GetValue())
  {

    if (App->EditValue->GetValue() == 2)
    {

      App->EditMode->SetValue(false);
      App->EditValue->SetValue(0);

      App->Settings.Temp = App->DestTemp->GetValue();
      App->Settings.Hum = App->DestHumidity->GetValue();
      EEPROM.put(0, App->Settings);
    }
    else
    {

      App->EditValue->SetValue(App->EditValue->GetValue() + 1);
    }
  }
  else
  {

    App->EditMode->SetValue(true);
    App->EditValue->SetValue(1);
  }

  DrawMenu();
}

void OnRight(TEncoder *Encoder)
{

  if (App->EditMode->GetValue())
  {

    if (App->EditValue->GetValue() == 1)
    {

      if (App->DestTemp->GetValue() > 1)
        App->DestTemp->SetValue(App->DestTemp->GetValue() - 1);
    }
    else if (App->EditValue->GetValue() == 2)
    {

      if (App->DestHumidity->GetValue() > 1)
        App->DestHumidity->SetValue(App->DestHumidity->GetValue() - 1);
    }

    DrawMenu();
  }
};



void OnLeft(TEncoder *Encoder)
{
  if (App->EditMode->GetValue())
  {

    if (App->EditValue->GetValue() == 1)
    {

      if (App->DestTemp->GetValue() < 60)
        App->DestTemp->SetValue(App->DestTemp->GetValue() + 1);
    }
    else if (App->EditValue->GetValue() == 2)
    {

      if (App->DestHumidity->GetValue() < 99)
        App->DestHumidity->SetValue(App->DestHumidity->GetValue() + 1);
    }

    DrawMenu();
  }
};

void CheckHeat(){

  bool State = (App->dht->Temperature()<App->Settings.Temp) || (App->dht->Humidity() > App->Settings.Hum);
  App->HeaterOn->SetValue(State);
  digitalWrite(9,!State);

}

void Timer1_OnTimeout(TTimer *Timer)
{

  float T, H;

  App->dht->UpdateValues();
  T = App->dht->Temperature();
  H = App->dht->Humidity();

  CheckHeat();

  DrawMenu();

  Serial.println("T = " + String(T));
  Serial.println("H = " + String(H));
};

unsigned long int TimerSource()
{
  return millis();
};

void setup()
{

  GetTimerValue = TimerSource;
  Serial.begin(9600);

  App = new TApp();
  App->Run();

  Serial.println("App started!");
}

void loop()
{

  App->Idle();
}
