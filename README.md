# 🍏 Apple ADB Ressurector

![PlatformIO](https://img.shields.io/badge/platform-PlatformIO-orange)
![Licence](https://img.shields.io/badge/licence-GNU%20GPL%20v3-blue)
![Status](https://img.shields.io/badge/status-Beta-yellow)

**Apple ADB Ressurector** : Donnez une seconde vie à vos trésors vintage ! 🕰️✨

Transformez vos claviers et souris Apple Desktop Bus (ADB) des années 80 et 90 en périphériques USB modernes avec ce projet passionnant ! 🎉
Compatible même avec les légendaires souris à un seul bouton, ce projet utilise PlatformIO pour donner une nouvelle vie à vos reliques de la pomme arc en ciel. 🖱️✨

---

## 🔗 Basé sur le travail de

Ce projet est basé sur le travail initial de [Szymon Łopaciuk](https://github.com/szymonlopaciuk/stm32-adb2usb). Vous pouvez consulter son dépôt ici : [stm32-adb2usb](https://github.com/szymonlopaciuk/stm32-adb2usb). Merci à lui pour son incroyable contribution à la communauté open source ! 🙌

---

## 🌐 Mon dépôt

Le code source de ce projet est disponible sur mon dépôt GitHub : [Apple ADB Ressurector](https://github.com/electron-rare/Apple-ADB-Ressurector). N'hésitez pas à y jeter un œil, à contribuer ou à poser des questions ! 😊

<img src="./assets/project-setup.jpeg" alt="Setup du projet" width="800">  
*Un aperçu de mon setup avec un clavier Apple d'époque et une souris ADB.*

---

## 🚀 Installation

1. Clonez ce magnifique dépôt :
   ```bash
   git clone https://github.com/electron-rare/Apple-ADB-Ressurector.git
   cd apple-adb-ressurector
   ```

2. Installez PlatformIO (si ce n'est pas déjà fait) :
   ```bash
   pip install platformio
   ```

3. Compilez et téléversez le firmware sur votre STM32 :
   ```bash
   platformio run --target upload
   ```

---

## 🎮 Utilisation

1. Téléversez le firmware sur votre carte STM32 ou ESP32.
2. Branchez votre périphérique ADB à la carte STM32 ou ESP32 (selon votre configuration).  
3. Ajoutez une résistance de pull-up entre la broche de données (ADB_PIN) et l'alimentation (+V). Voici un schéma de connexion :  
```
                             +5V
                             |
                             R (4.7kΩ ou 10kΩ)
                             |
Périphérique ADB data <------+------------------+---> ADB_PIN (ESP32: GPIO2, STM32: PB4)
                             |
                             |
                            GND
```
4. Connectez la carte STM32 ou ESP32 à votre ordinateur via USB ou Bluetooth.

4. **Utilisation de connecteurs et câbles S-Video** :  
   Le connecteur ADB est identique au connecteur S-Video. Vous pouvez donc utiliser des câbles S-Video standard pour connecter vos périphériques ADB.  

5. Admirez la magie : votre périphérique ADB fonctionne maintenant comme un périphérique USB ou Bluetooth ! ✨  

> **Note** : Grâce à l'environnement PlatformIO et à la configuration du firmware, tout le travail complexe est automatisé. Vous n'avez qu'à choisir la plateforme (STM32 ou ESP32) et à téléverser le firmware.  

---

## 🌟 Fonctionnalités actuelles

- **Clavier USB HID** : Conversion des touches ADB en rapports HID USB, avec gestion des modificateurs (Shift, Ctrl, etc.) et des touches spéciales (Caps Lock, Num Lock).  
- **Souris USB HID** : Conversion des mouvements et clics ADB en rapports HID USB.  
- **Gestion des LEDs** : Les LEDs Caps Lock et Num Lock fonctionnent comme par magie.  
- **Compatibilité HID** : Utilisation de `HID_Composite` pour gérer les rapports HID.  

---

## 📚 Bibliothèque ADB-pour-Framework-Arduino

Ce projet repose sur la bibliothèque [ADB-pour-Framework-Arduino](https://github.com/electron-rare/ADB-pour-Framework-Arduino), créée pour améliorer la portabilité et permettre une utilisation sur plusieurs plateformes (STM32, ESP32, Arduino AVR, Teensy).  

### Pourquoi cette bibliothèque ?  
Parce que je m'ennuyais, et que je voulais explorer les possibilités de GitHub Copilot pour compenser mes modestes compétences en programmation. Résultat : une bibliothèque qui fonctionne (presque) parfaitement et qui me fait passer pour un génie du code. Merci Copilot ! 🤖✨

- **Portabilité** : Compatible avec plusieurs plateformes.  
- **Modularité** : Réutilisable dans d'autres projets.  
- **Facilité de maintenance** : Centralisation des mises à jour.  

---

## 🤔 Pourquoi ce projet ?

Parce que je voulais retrouver le plaisir d'utiliser un ancien clavier Apple (et que j'avais ça qui traînait parmi tout mon bazar). Mais évidemment, il fallait bien le rendre compatible avec nos machines modernes. Alors, sur mon temps libre, j'ai fouillé sur le net pour voir ce que d'autres avaient déjà fait (parce qu'on est rarement le premier, soyons honnêtes !).  

Mais voilà, je ne trouvais pas exactement ce que je voulais, et comme j'avais des devkits qui prenaient la poussière, je me suis dit : "Pourquoi pas ?" Résultat : ce projet est né, réalisé et partagé pendant mes moments de libre. Parce que, après tout, c'est toujours plus sympa de redonner vie à ces vieux périphériques d'époque que de les laisser dormir dans un carton. 😄

---

## ⌨️ Claviers compatibles

- **Apple Keyboard (1987)** : Le clavier standard avec pavé numérique intégré. Simple, efficace, rétro.  
- **Apple Extended Keyboard (1987–1990)** : Le Saint Graal des claviers mécaniques Apple.  
- **Apple Extended Keyboard II (1990–1994)** : Une version améliorée, élégante et toujours aussi robuste.  
- **Apple Adjustable Keyboard (1993)** : Le clavier ergonomique avant-gardiste... mais un peu encombrant.  

---

## 🖱️ Souris compatibles

- **Original ADB Mouse (1986)** : La souris rectangulaire avec un seul bouton. Minimalisme à son apogée.  
- **ADB Mouse II (1993)** : Une version plus ergonomique, avec un bouton plus grand et une meilleure résolution.  

---

## 🛠️ Autres périphériques compatibles

- **Tablettes graphiques** : Wacom ADB, Kurta ADB.  
- **Trackballs** : Kensington Turbo Mouse, Microspeed MacTRAC.  
- **Joysticks** : Advanced Gravis MouseStick II.  
- **Lecteurs de codes-barres** : Datalogic Heron D130.  
- **Claviers alternatifs** : IntelliKeys.  

---

## ⚙️ Configuration des pins et paramètres

Le projet utilise des définitions spécifiques pour configurer les pins en fonction de la plateforme utilisée (ESP32 ou STM32) :

- `#define POLL_DELAY 5` : Définit un délai de 5 ms entre chaque cycle de polling pour interroger les périphériques ADB.  
- `#define ADB_PIN` : Configure la pin utilisée pour la communication ADB :
  - **ESP32** : Pin `2`.  
  - **STM32** : Pin `PB4`.  
- `#define LED_PIN` : Configure la pin utilisée pour la LED d'état :
  - **ESP32** : Pin `4` (Devkit Wemos).  
  - **STM32** : Pin `PC13`.  

Ces définitions permettent une compatibilité multi-plateforme en adaptant automatiquement les pins selon la carte utilisée.

---

## 🛑 État du projet

Ce projet est actuellement en **beta**.  
- **ESP32** : Le support du clavier Bluetooth est désormais fonctionnel. Prochain objectif : ajouter le support Bluetooth pour la souris.  
- **STM32** : Le support du clavier et de la souris comme périphériques USB est entièrement fonctionnel.  

---

## 🔮 Plans futurs

- Concevoir un PCB pour une alimentation par batterie et un boîtier adapté.
- Ajouter le support Bluetooth pour la souris.
- Intégrer un écran OLED pour afficher des informations sur l'état de la connexion.
- Ajouter un mode de veille pour économiser la batterie.

---

## 📖 Ressources utiles

- [Guide to the Macintosh&copy; Family Hardware (2nd ed.)](https://archive.org/details/apple-guide-macintosh-family-hardware)  
- [ADB - The Untold Story](https://developer.apple.com/library/archive/technotes/hw/hw_01.html#//apple_ref/doc/uid/DTS10002470)  
- [Inside Macintosh volume V](https://archive.org/details/InsideMacintoshVolumeV)  

---

## 📝 TODO

- [x] Faire fonctionner le clavier.  
- [x] Faire fonctionner la souris.  
- [x] Gérer plusieurs périphériques ADB simultanément.
- [x] Ajouter le support Bluetooth pour le clavier.  
- [ ] Ajouter le support Bluetooth pour la souris.  
- [ ] Ajouter le support d'autres périphériques ADB (tablettes graphiques, trackballs, etc.).
- [ ] Ajouter le support d'un écran OLED pour afficher des informations sur l'état de la connexion.
- [ ] Ajouter un mode de veille pour économiser la batterie.
- [ ] Concevoir un PCB pour une alimentation par batterie et un boîtier adapté.
- [ ] Améliorer le support de la LED Num Lock.
- [ ] Améliorer le support de la LED Caps Lock.
- [ ] Améliorer le support de la LED Scroll Lock.


---

## 💡 Remerciements

Un grand merci à **GitHub Copilot** pour son aide précieuse et ses suggestions parfois hilarantes. 🤖✨  
Et bien sûr, surtout merci à **Szymon Łopaciuk** pour l'inspiration initiale.  

---

## 📜 Licence

Ce projet est sous licence GNU GPL v3. Voir le fichier [LICENSE](LICENSE) pour plus de détails.  

---
