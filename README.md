Bonjour Voice le read me 

Installation et lancement

Extrayez tous les fichiers de l'archive ZIP.
Vérifiez que les dossiers shaders et textures sont présents à côté de l'exécutable.
Assurez-vous que votre système possède un pilote graphique compatible Vulkan.
Si nécessaire, installez le Microsoft Visual C++ Redistributable adapté.
Lancez Vulkan-Cube.exe.
Configuration requise

Windows x64
GPU et pilote compatibles avec les fonctionnalités Vulkan requises par l'application
Runtime Visual C++ compatible, si nécessaire
Ressources utilisées

shaders/slang.spv
textures/Maxwelle.jpg

Vulkan Triangle
1. Présentation du projet
Ce projet est une application graphique développée en C++ avec l'API Vulkan. Il s'appuie sur les bases du tutoriel Vulkan consacré au rendu d'un triangle, puis les étend à l'affichage d'un cube texturé en rotation.

L'objectif est de comprendre le fonctionnement du pipeline graphique Vulkan, la gestion des ressources GPU, le chargement des textures et la présentation des images à l'écran.

Le rendu utilise un cube composé de sommets et d'indices, une texture chargée depuis un fichier image et des matrices de transformation mises à jour à chaque image.

2. Fonctionnalités
Initialisation de l'instance Vulkan.
Détection et sélection d'un périphérique graphique compatible.
Création d'une fenêtre d'affichage.
Mise en place de la surface et de la swapchain.
Création du pipeline graphique.
Chargement des shaders compilés.
Rendu d'un triangle à l'écran.
Gestion de la boucle principale et de l'affichage des images.
Les fonctionnalités listées ci-dessus sont à vérifier par rapport à l'implémentation réelle du projet.

3. Environnement et versions
Le projet nécessite les outils suivants :

Vulkan SDK	1.4.357.0
C++ C++23
Compilateur C++	Visula studio
Système d'exploitation : Windows 

Carte graphique : un GPU et un pilote compatibles avec Vulkan sont nécessaires pour que sa marche.

4. Installation:
   
Installer le Vulkan SDK
Télécharger et installer le Vulkan SDK depuis le site officiel :

https://www.lunarg.com/vulkan-sdk/

5. Compilation des shaders
Les shaders décrivent certaines étapes du pipeline graphique.

Le projet utilise deux shaders :

Vertex shader (.vert) : définit la position des sommets et transmet les données nécessaires aux étapes suivantes.
Fragment shader (.frag) : détermine la couleur des fragments générés par la rastérisation.


6. Limitation7 no dans le project :

Absence de système de caméra
Gestion du redimensionnement de la fenêtre   


7. Exécution :

 lancer le programme et vous devrai avoir un cube qui tourne automatiquement 
