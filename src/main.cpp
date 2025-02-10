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
int StateH = 0;

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

    EditValue = new TInt();
    EditValue->SetValue(0);

    DestTemp = new TFloat();
    DestHumidity = new TFloat();
    DestTemp->SetValue(Settings.Temp);
    DestHumidity->SetValue(Settings.Hum);

    pinMode(9,OUTPUT);
    digitalWrite(9,1);

    //Serial.println("DEST TEMP : " + String(Settings.Temp));
    //Serial.println("DEST HUM : " + String(Settings.Hum));
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

    if(StateH<0){
      App->Screen->setTextSize(1);
      App->Screen->setCursor(0, 21);
      App->Screen->print("!");
    }
    if(StateT<0){
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


void OnLeft(TEncoder *Encoder)
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



void OnRight(TEncoder *Encoder)
{
  if (App->EditMode->GetValue())
  {

    if (App->EditValue->GetValue() == 1)
    {

      if (App->DestTemp->GetValue() < 65)
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

  float H = App->dht->Humidity(), Hs = App->Settings.Hum;
  
  StateH = 0;
  StateT = 0;

  if( H < Hs ) StateH = -2;
  if( H > (Hs + 1)) StateH = 1;

  float T = App->dht->Temperature(), Ts = App->Settings.Temp;

  if( T > Ts ) StateT = -2;
  if( T < (Ts - 1)) StateT = 1;
  
  int State = StateH + StateT;

  if(State>0){
    App->HeaterOn->SetValue(true);
    digitalWrite(9,false);
  }
  else if(State<0){
    App->HeaterOn->SetValue(false);
    digitalWrite(9,true);
 }
//Serial.println(" H="+String(H) + " Hs="+String(Hs)+" Sh="+String(StateH));
//Serial.println(" T="+String(T) + " Ts="+String(Ts)+" ST="+String(StateT));
//Serial.println(" STATE="+String(State));
//Serial.println();
  

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

  //Serial.println("App started!");
}

void loop()
{

  App->Idle();
}
