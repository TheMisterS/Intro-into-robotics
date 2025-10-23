# Arduino powered 2FA(pseudo) 

## Problem
There is a need to create an authentication system for greenhouses, it has to satisfy the following requirements:
- Only people who know the password may enter.
- Entrance may only be granted during the day, so no one is able to access it off-work hours.

## Design
Design is oriented around a photoresistor and a keypad. 

The first authentication stage happens via the keypad, where an entered password is compared against a set one. If the first stage is successful, then the second one occurs, where a photoresistor's signal, gathered from the strength of light hitting it, is evaluated against a set threshold.

If both of the stages are succesfull, the servo attached to a door locked is powered.

## Parts List
List of components used in the project:
| Name     | Quantity | Component                |
|----------|----------|--------------------------|
| U1       | 1        | Arduino Uno R3           |
| D1       | 1        | Red LED                  |
| R1, R2   | 2        | 1 kΩ Resistor            |
| KEYPAD2  | 1        | Keypad 4x4               |
| M1       | 1        | DC Motor                 |
| D2       | 1        | Diode                    |
| T1       | 1        | NPN Transistor (BJT)     |
| R3       | 1        | Photoresistor            |
| R4       | 1        | 10 kΩ Resistor           |

[View CSV](./illustrations_and_extras/bom.csv)

## Schematics
<img alt="image" src="./illustrations_and_extras/pseudo_2fa_mechanism.png" />
<img alt="image" src="./illustrations_and_extras/pseudo_2fa_mechanism_schematic.png" />

[View PDF](./illustrations_and_extras/pseudo_2fa_mechanism.pdf)

## Demo

https://github.com/user-attachments/assets/ba1d6b73-0267-491b-9caa-456fdc94bc86

[View uncompressed mp4](././illustrations_and_extras/pseudo_2fa_mechanism.mp4)

## Future Improvements

- Add more visual/auditory  indication to the system user whether the password entered is correct/wrong/clear.
- Add more factors, either as an additional security measure or an alternative for one of the stages.
- Add a Wi-Fi module to apply configuration changes remotely. Perhaps a data layer to manage users and their access information.
