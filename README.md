# Firmware for GPIO Extender-T (TOIC)

Инструкция по прошивке и проверке GPIO Extender-T

1. Подключить устройство к ПК. Убедиться, что появилось новое устройство /dev/ttyACM*. Вызвать
команду

```
udevadm info /dev/ttyACM0 | grep ID_MODEL=
```

Если значение `ID_MODEL=USB_GPIO_Extender`, то перейти к шагу тестирования (3). Если
`ID_MODEL=TOIC-F0-GE`, то устройство требуется предварительно прошить.

2. Прошивка устройства

    1. Скачать ToicIDE ([ссылка](https://software.open-dev.ru/software/toicide/0.2.2/ToicIDE-x64.AppImage)).
    Добавить разрешение на исполнение `chmod +x <PATH_TO_IDE>`.

    2. Запустить IDE от root пользователя. Создать новый проект. В board выбрать `sys://STM32F0.tbsp`.

    3. Скопировать код прошивки из `fw.tcc` в текстовый редактор IDE. Выполнить сборку (Ctrl+B).

    4. В левом верхнем углу IDE нажать кнопку Connect, выбрать HID устройство.

    <img src="_img\connect-to-device.png" alt="connect_to_device" align="center">

    Кнопка Connect должна изменить цвет с красного на зеленый.

    5. Нажать кнопку "Push script to the device" для прошивки. Дождаться окончания прошивки.
    На устройстве должен начать мигать красный светодиод. Извлечь устройство из ПК.

3. Тестирование устройства

    1. Подключить выводы устройства согласно рисунку.

    <img src="_img\extender-test-connection.png" alt="test_connection" align="center">

    **Прим**. Вывод 10 (PA14) не программируется. При попытке задать конфигурацию устройство
    вываливается в исключение и перестает работать. Плюс в описании устройства в IDE указано, что
    10 вывод связан с PA5, поэтому приходится соединять для проверки так.

    2. Подключить устройство к ПК. Запустить скрипт `usb-gpio-extender-check.py`. В случае отсутствия
    ошибок считается, что устройство запрограммированно корректно и готово к работе.

