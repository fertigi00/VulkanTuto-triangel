Bonjour Voice le read me 

si vous lancer le program un cube en 3D apparaitra sur une feunaitre Windows et tournera automatique ment avec une image de cu

Vulkan Triangle
1. Présentation du projet
Ce projet a pour objectif de découvrir les bases de l'API graphique Vulkan à travers l'affichage d'un cube qui peux tourner dans une fenêtre.

L'application est développée en C++ et utilise des shaders pour exécuter les étapes programmables du pipeline graphique. Elle permet de comprendre les principales étapes nécessaires à la création d'un rendu avec Vulkan.

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
