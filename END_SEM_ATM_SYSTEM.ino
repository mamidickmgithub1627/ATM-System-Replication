//Accesing the ESP32 in station mode.
//required libraries
#include<WiFi.h>
#include <EEPROM.h>
//Library to use EEPROM memory
#include<WiFiClientSecure.h>
//For telegram bot
#include<UniversalTelegramBot.h>// Telegram chatbot library
#include<ArduinoJson.h>
#define Eeprom_size 4//for 3 denominations and for total.



/*
int Amount  = 25000;//Available amount
int Denom_two_thous = 5;
int Denom_thous = 10;
int Denom_five_hundred = 10;
*/

// will be updated to 1 when user is authenticated.
int user_verified = 0 ; 

//Wifi creditinal details.
const char* my_ssid = "Mani_wifi";
const char* my_pass = "manikanta";

//Token of the telegram bot
#define BOT_tok "2108228709:AAFIHnrgD34O_C8SgsAF_CWF9zX2gNZ8VWU"

WiFiClientSecure client;
//defining bot.
UniversalTelegramBot bot(BOT_tok,client);


void Handle_newly_recieved_Messages(int Msg_Counter){
  int i;
  for(i=0;i<Msg_Counter;i++){
    String chat_id=String(bot.messages[i].chat_id);
    /*if(chat_id !"){
      Serial.println("Unauthorized User!!!!!!");
      continue;
    }*/
    String msg_recieved= bot.messages[i].text;
    String User = bot.messages[i].from_name;
    Serial.println("Message from "+User+" : "+msg_recieved);
    if(msg_recieved=="/start"){
      String welcome_msg = "Welcome "+User+".\n";
      welcome_msg+="/login to login to your account";
      welcome_msg+="\n /check_balance for checking balance\n /withdraw for Money withdrawl \n /logout to logout your account";
      bot.sendMessage(chat_id,welcome_msg,"");
    }



    //condition to check tour balance
    if(msg_recieved=="/check_balance"){
      if (user_verified)
      {
        String bal = "Your Current Balance is";
        int value = EEPROM.read(0)*2000+EEPROM.read(1)*1000+EEPROM.read(2)*500;
        
        bal= bal+ String(value);
        bot.sendMessage(chat_id,bal,"");
      }
      else{
        String bal = "You are not logged in!, Login to view your blance!";
        bot.sendMessage(chat_id,bal,"");
        }
    }



    //condition to Logout the account
    if(msg_recieved=="/logout"){
      user_verified = 0;
      String log_out = "You are Logged out, Press login to login again";
      bot.sendMessage(chat_id,log_out,"");
    }



    //condition to withdraw amount from ATM
    if(msg_recieved=="/withdraw"){
      if (user_verified)
      {
         String bal = "Your Current Balance is";
         int value = EEPROM.read(0)*2000+EEPROM.read(1)*1000+EEPROM.read(2)*500;
         bal= bal+ String(value);
         bal = bal+"\n Enter no of 2000's, 1000's and 500's you want to withdraw in BCD 12 for every denomination, i.e., totaly you have to enter 4x3 = 12 bits, with 1 sec delay.";
         bal = bal+"\n Example: For 5000, you can enter\n 2*2000 +1000 [0010 0001 0000] or\n 5* 1000 [0000 0101 0000] or 10*500 [0000 0000 1010] ";
         bot.sendMessage(chat_id,bal,"");
         int trans[12];// Transaction ammount stored , given by user bit by bit.
         int itr=0;//transaction bit variable index
         int bit1=120,bit0=120;
         while(itr<12){
            bit1 = touchRead(12);// reads the touch sensititvity value.
            bit0 = touchRead(13);
            if (itr>11)
            break;
            if (bit1<40){
              Serial.println("Entered 1");
              Serial.println(itr);
              digitalWrite(2,HIGH);
              delay(100);
              digitalWrite(2,LOW);
              trans[itr]=1;
              itr++;
            }
            else if (bit0<40)
            {
              Serial.println("Entered 0");
              Serial.print(itr);
              digitalWrite(2,HIGH);
              delay(100);
              digitalWrite(2,LOW);
              trans[itr]=0;
              itr++;
            }
            delay(1000);
          }
         int n1=0,n2=0,n3=0; //3 denominations; no of each denominations user want.
         int j,temp;
         //Decoding for no of 2000's from BCD to decimal
         for (j=3;j>=0;j--)
         {
            temp = pow(2,3-j);
            n1=n1+trans[j]*temp; 
          }
        //sum*=10;
        //Decoding for no of 1000's from BCD to decimal
         for (j=7;j>=4;j--)
         {
            temp = pow(2,7-j);
            n2=n2+trans[j]*temp; 
         }
         //Decoding for no of 500's from BCD to decimal
         for (j=11;j>=8;j--)
         {
          temp = pow(2,11-j);
          n3=n3+trans[j]*temp; 
         }
         //checking whether user required denominations available or not!
         if (n1 <= EEPROM.read(0) && n2 <=EEPROM.read(1)&& n3 <= EEPROM.read(2))
         {
          int debit_ammount;
          debit_ammount = n1*2000+n2*1000+n3*500;
          //Updating the denominations and total available ammount.
          int value = EEPROM.read(0)*2000+EEPROM.read(1)*1000+EEPROM.read(2)*500;
          value = value - debit_ammount;
          bot.sendMessage(chat_id,"Remaining Balance is: "+String(value),"");
          int remain = EEPROM.read(0)-n1;
          EEPROM.write(0,remain);
          remain = EEPROM.read(1)-n2;
          EEPROM.write(1,remain);
          remain = EEPROM.read(2)-n3;
          EEPROM.write(2,remain);
          /*
          Denom_two_thous = Denom_two_thous-n1 ;
          Denom_thous = Denom_thous-n2;
          Denom_five_hundred = Denom_five_hundred-n3;
          */
          }
         else
         {
          String not_available = "Denominations you entered are not available!";
          not_available= not_available+ "\n Transaction Failed!, Try again!";
          bot.sendMessage(chat_id,not_available,"");
          }

      }
      else {
        String debit  = "You are not logged in!, Login to withdraw amount!";
        bot.sendMessage(chat_id,debit,"");
        }
    }
    




    //Condition to login for the user.
    if(msg_recieved=="/login" && !user_verified){
      int num;
      //Random OTP Genration using random function!
      num = random(0,99);
      String s = "Your login OTP is: ";
      s = s+ String(num);
      s+= "\n Enter it as BCD equivalent with 1 sec delay.\n Suppose if number is 23, Enter 0010 0011 each bit with 1 sec delay.";
      s+="\n If you touch pin 12, considered as 1, otherwise if you touch pin 13 Considered as 0";
      bot.sendMessage(chat_id,s,"");
      // array for BCD equivalent of 2 digit OTP.
      int arr[8];
      int i1=0;
      int val=120,val1=120;
      while(i1<8){
          val = touchRead(12);
          val1 = touchRead(13);
          if (i1>7)
          break;
          if (val<40){
            Serial.println("Entered 1");
            Serial.println(i1);
            digitalWrite(2,HIGH);
            delay(100);//blinking LED to show usert that the entered bit is taken.
            //It helps user to move to next bit.
            digitalWrite(2,LOW);
            arr[i1]=1;
            i1++;
           }
          else if (val1<40)
          {
            Serial.println("Entered 0");
            Serial.print(i1);
            digitalWrite(2,HIGH);
            delay(100);
            digitalWrite(2,LOW);
             arr[i1]=0;
             i1++;
           }
           delay(1000);
        }
      int sum =0;
      int j,temp;
      for (j=3;j>=0;j--)
      {
        temp = pow(2,3-j);
        sum+=arr[j]*temp; 
      }
      // to place first BCD equivalent decimal in 10's place!
      sum*=10;
      
      for (j=7;j>=4;j--)
      {
        temp = pow(2,7-j);
        sum+=arr[j]*temp; 
      }
    //Serial.print("Here");
      if (sum == num)
      {
        s = "You are Verified Now!";
        // string to update status
        bot.sendMessage(chat_id,s,"");
        user_verified = 1;
      }
      else
      {
        s = "OTP Incorrect Try Login again";
        //string to update status.
        bot.sendMessage(chat_id,s,""); 
      }
        
    }
    else if(msg_recieved=="/login" ){
        String log_in = "You are already Logged in, Please logout to login again!";
        //string to update status.
        bot.sendMessage(chat_id,log_in,"");     
      }
  }
}


void setup() {
  // put your setup code here, to run once:
  EEPROM.begin(Eeprom_size);
  EEPROM.write(0,5);//No of 2000's
  EEPROM.write(1,10);//No of 1000's
  EEPROM.write(2,10);//No of 500's
  Serial.begin(9600);//baudrate
  pinMode(2,OUTPUT);
  client.setInsecure();
  //setting esp32 in station mode
  WiFi.mode(WIFI_STA);
  //disconnecting if already connected
  WiFi.disconnect();
  //pinMode(GPIO_PIN,OUTPUT);
  WiFi.begin(my_ssid,my_pass);
  Serial.println("Connecting!");
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status()==WL_CONNECTED)  {
    int newMessagesCount = bot.getUpdates(bot.last_message_received + 1);
    //count if any new messages
    //Serial.println(newMessagesCount);
    while(newMessagesCount) {
      Serial.println("got response");
      //passing new messages count to handle new messages.
      Handle_newly_recieved_Messages(newMessagesCount);
      newMessagesCount = bot.getUpdates(bot.last_message_received + 1);
    }
    //Serial.println("*");
    delay(500);
  }

}
