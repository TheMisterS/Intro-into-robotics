// int a = 5
// int b = 6
// int c = 9
// int d = 8
// int e = 7
// int f = 4
// int g = 3
// int dp = 10


int test = 10; 

void setup() {
  pinMode(test, OUTPUT);
  digitalWrite(test, HIGH);
}

void loop() {
  digitalWrite(test, LOW); 
  delay(1000);
  digitalWrite(test, HIGH); 
  delay(1000);
}
