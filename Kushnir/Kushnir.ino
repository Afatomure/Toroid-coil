const int line_finder_pin = 3;                                    // Датчик линии на 3 пин
int turns = 0;                                                    // Количество оборотов (изначально 0)
int need_turns;                                                   // FixMe: Целевое количество оборотов, должно выставляться кнопками или крутилкой
bool is_black_before = false;                                     // Белый цвет на прошлой итерации цикла
byte screen_mode = 3;                                             // По умолчанию выставлен первый экран, в котором предлагается установить число оборотов.

void setup() {
  pinMode(line_finder_pin, INPUT);                                // Инициализируем датчик линии
  Serial.begin(9600);
  need_turns = 15;
}

void loop() {

  // Сначала открываем меню 1 или 2, между ними можно переключаться
  // Потом, когда нажали start, началась намотка, запускается while ниже и экран 3
  
  switch(screen_mode){
    case 1: Serial.println("Экран 1");
    case 2: Serial.println("Экран 2");
    case 3:
      while (turns != need_turns){
        if (digitalRead(line_finder_pin) == HIGH && !is_black_before){  // Если был белый, а теперь черный (т.е. изолента прошла круг), то
          turns++;                                                      // увеличиваем число оборотов на 1
          is_black_before = true;                                       // и меняем флаг, что был черный
          Serial.println(turns);
        }
        if (!digitalRead(line_finder_pin) == HIGH){
          is_black_before = false;
        }
        delay(50);                                                      // ToDo: Задержку подогнать под скорость вращения тороида...
      }
      screen_mode = 4;
    case 4:
      // Когда намотка закончилась, выводим экран 4, а на нём что-то типа:
      Serial.println("Намотка закончена!");
  }
}
