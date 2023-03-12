#include <LiquidCrystal.h>
#include <EncButton.h>
#include <Stepper.h>

// Подключаем дисплей 16х2
constexpr uint8_t PIN_RS = 6;
constexpr uint8_t PIN_EN = 7;
constexpr uint8_t PIN_DB4 = 8;
constexpr uint8_t PIN_DB5 = 9;
constexpr uint8_t PIN_DB6 = 10;
constexpr uint8_t PIN_DB7 = 11;
LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_DB4, PIN_DB5, PIN_DB6, PIN_DB7);
byte screen_mode = 1; // По умолчанию выставлен первый экран, в котором предлагается установить число оборотов.

// Подключаем энкодер
const uint8_t DT = 2;
const uint8_t CLK = 3;
const uint8_t SW = 4;
EncButton<EB_TICK, DT, CLK, SW> enc;  // энкодер с кнопкой

const int line_finder_pin = 5; // Датчик линии (тахометр) на 5 пин

int16_t turns = 0; // Количество необходимых витков (изначально и минимально: 0, максимально: ~32500) - оно же количество шагов для полного оборота шаговика
int16_t angle = 30; // Угол между витками, влияет на плотность намотки (по умолчанию 30 градусов)
int current_turns = -1; // Текущее количество оборотов
bool is_black_before = false; // Белый цвет на прошлой итерации цикла

Stepper myStepper(200, A3, A2, A1, A0); // Создаем объект шаговика (200 - это количество шагов, необходимых для одного полного оборота шаговика, т.е. 1,8 градуса - минимуальный поворот)

void setup() {
  pinMode(line_finder_pin, INPUT); // Инициализируем датчик линии

  myStepper.setSpeed(60); // Устанавливаем скорость вращения 60 об./мин.= 1 об./сек = 200 шагов / сек

//  Serial.begin(9600);
  enc.setEncType(EB_HALFSTEP);  // тип энкодера: EB_FULLSTEP (0) по умолч., EB_HALFSTEP (1) если энкодер делает один поворот за два щелчка
  lcd.begin(16, 2);
  Screen_1();
}

void loop() {

  // Сначала открываем меню 1 или 2, между ними можно переключаться
  // Потом, когда нажали start, началась намотка, запускается while ниже и экран 3

  switch (screen_mode) {

    case 1:
      enc.tick(); // опрос происходит здесь
      //Serial.println("Экран 1");
      if (enc.left()) {
        turns--;     // если энкодер влево, то уменьшаем число оборотов (мин. 0)
        Screen_1(); // включаем первый экран
      }
      if (enc.leftH()) {
        turns = turns + 10;
        Screen_1();
      }
      if (turns <= 1) turns = 1;
      if (enc.right()) {
        turns++;   // если энкодер вправо, то увеличиваем число оборотов (макс. 32500)
        Screen_1(); // включаем первый экран
      }
      if (enc.rightH()) {
        turns = turns - 10;
        Screen_1();
      }
      if (enc.click()) {
        screen_mode = 2;   // если энкодер был нажат, то переключаемся на экран 2
        Screen_2();       // включаем второй экран
      }

      break;

    case 2:
      enc.tick(); // опрос происходит здесь
      // Serial.println("Экран 2");
      if (enc.left()) {
        angle--;     // если энкодер влево, то уменьшаем угол (мин. 0)
        Screen_2(); // включаем второй экран
      }
      if (enc.leftH()) {
        angle = angle + 10;
        Screen_2();
      }
      if (angle <= 1) angle = 1;
      if (enc.right()) {
        angle++;   // если энкодер вправо, то увеличиваем число оборотов (макс. 360)
        Screen_2(); // включаем второй экран
      }
      if (enc.rightH()) {
        angle = angle - 10;
        Screen_2();
      }
      if (angle >= 360) angle = 360;
      if (enc.click()) {
        
        screen_mode = 3;   // если энкодер был нажат, то переключаемся на экран 3
        Screen_3(); // включаем третий экран
        break;
      }
      if (enc.held()) {
        screen_mode = 1;   // если энкодер был нажат с удержанием, то переключаемся на экран 1
        Screen_1(); // включаем первый экран
      }
      break;

    case 3:
      while (turns != current_turns) {

        /*
         * Вот сюда дописать код, отвечающий за работу мотора диска
         */
        
        myStepper.step(map(angle, 0, 359, 0, 200)); // поворот шаговика на заданный в экране 2 угол
        
        if (digitalRead(line_finder_pin) == HIGH && !is_black_before) { // Если был белый, а теперь черный (т.е. изолента прошла круг), то
          current_turns++;                                              // увеличиваем число оборотов на 1
          is_black_before = true;                                       // и меняем флаг, что был черный
        }
        if (digitalRead(line_finder_pin) == LOW) {
          is_black_before = false;
        }

        enc.tick(); // опрос энкодера
        if (enc.click()) {
          break;
        }
        
        
        //Serial.println(current_turns);
        delay(50);                                                      // ToDo: Задержку подогнать под скорость вращения диска
        Screen_3();
      }
      screen_mode = 4;
      Screen_4();
      current_turns = 0;
      break;

    case 4:
      enc.tick(); // опрос происходит здесь
      // Когда намотка закончилась, выводим экран 4, а на нём что-то типа:
      //Serial.println("Намотка закончена!");
      if (enc.click()) {
        screen_mode = 1;   // если энкодер был нажат, то переключаемся на экран 2
        Screen_1();
        break;
      }
      break;
  }
}

void Screen_1() {
  lcd.print("                "); // очистка дисплея
  lcd.setCursor(0, 0); // Устанавливаем курсор в колонку 0 и строку 0
  // lcd.print("  Число витков  "); // Печатаем первую строку
  lcd.print("Num of turns    "); // Печатаем первую строку
  lcd.setCursor(0, 1);
  // lcd.print("Витков: ");
  lcd.print("Turns: ");
  lcd.print(turns);
  lcd.print("        ");
}

void Screen_2() {
  lcd.print("                "); // очистка дисплея
  lcd.setCursor(0, 0); // Устанавливаем курсор в колонку 0 и строку 0
  // lcd.print("Плотность витков"); // Печатаем первую строку
  lcd.print("Turns density   "); // Печатаем первую строку
  lcd.setCursor(0, 1);
  // lcd.print("Угол: ");
  lcd.print("Angle: ");
  lcd.print(angle);
  lcd.print("        ");
}

void Screen_3() {
  lcd.print("                "); // очистка дисплея
  lcd.setCursor(0, 0); // Устанавливаем курсор в колонку 0 и строку 0
  // lcd.print("Сделано!"); // Печатаем первую строку
  lcd.print("Turns done: ");
  lcd.print(current_turns);
  lcd.print("   ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void Screen_4() {
  lcd.print("                "); // очистка дисплея
  lcd.setCursor(0, 0); // Устанавливаем курсор в колонку 0 и строку 0
  // lcd.print("Сделано!"); // Печатаем первую строку
  lcd.print("Done!           ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}
