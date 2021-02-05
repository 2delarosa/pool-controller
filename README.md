# 🏊 Smart Swimmingpool Controller 2.0

[![Smart Swimmingpool](https://img.shields.io/badge/%F0%9F%8F%8A%20-Smart%20Swimmingpool-blue.svg)](https://github.com/smart-swimmingpool)
[![PlatformIO CI](https://github.com/smart-swimmingpool/pool-controller/workflows/PlatformIO%20CI/badge.svg)](https://github.com/smart-swimmingpool/pool-controller/actions?query=workflow%3A%22PlatformIO+CI%22)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-v1.4%20adopted-ff69b4.svg)](code-of-conduct.md)
[![All Contributors](https://img.shields.io/badge/all_contributors-1-orange.svg?style=flat-square)](#contributors)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

[![works with MQTT Homie](https://homieiot.github.io/img/works-with-homie.svg "[works with MQTT Homie")](https://homieiot.github.io/)

**🏊 The Homie 3.0 compatible Smart Swimmingpool Controller 🎛️**

Adapted Stephan Strittmatter's (https://github.com/smart-swimmingpool/pool-controller) for my specific use.

## Main Features

- [x] [Homie 3.0](https://homieiot.github.io/) compatible MQTT messaging
- [x] Independent of specific smarthome servers
  - [x] [openHAB](https://www.openhab.org) since Version 2.4 using MQTT Homie
  - [x] [Home Assistant](home-assistant.io) using MQTT Homie
- [x] Can track up to five temperature sensors (Pool Suc/Ret, Spa Suc/Ret, Heater out)
- [x] Added a contact node for a water flow reed switch.  
- [x] Timesync via NTP (us.pool.ntp.org)
- [x] Logging-Information via Homie-Node
- [x] Disable automatic operation mode.  Default is manual operation

## Contributors

Thanks goes to these wonderful people
([emoji key](https://github.com/all-contributors/all-contributors#emoji-key)):

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore -->

<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the
[all-contributors](https://github.com/all-contributors/all-contributors)
specification. Contributions of any kind welcome!

## Credits

- Stephan Strittmatter (https://github.com/smart-swimmingpool/pool-controller)
- James Scott Jr. who worked on the temperature and contact homienode sensors (https://github.com/skoona/HomieDallasTemperatureNode)
- [Community of Homie-ESP8266](https://gitter.im/homie-iot/ESP8266)
- [Lübbe Onken](http://github.com/luebbe) for `TimeClientHelper`
- [Ian Hubbertz](https://github.com/euphi) for [HomieLoggerNode](https://github.com/euphi/HomieLoggerNode)

## License

[LICENSE](LICENSE)

---

Visit Stephan Strittmatter's DIY My Smart Home: (https://medium.com/diy-my-smart-home)
# pool-controller
