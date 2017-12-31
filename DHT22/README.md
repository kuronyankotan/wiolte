# wiolte example

## DHT22

Use:Grove - Temperature and Humidity Sensor Pro
http://wiki.seeed.cc/Grove-Temperature_and_Humidity_Sensor_Pro/

### grove-temperature-and-humidity-sensor22.ino
WioLTEのスケッチ例についてくる「grove-temperature-and-humidity-sensor」をDHT22用に修正したサンプルです。
基本的にはDHT11と同じくD38ポートで接続でき、上記サンプルで同じように湿度と温度（マイナス気温も対応）を取得出来ます。


### soracom_beam_temp_22.ino
温湿度センサーの「温度」が変化した時だけ
SORACOM beamを利用してデータを送信するサンプルです。
