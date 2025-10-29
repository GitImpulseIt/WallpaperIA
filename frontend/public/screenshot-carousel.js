// Carrousel 3D rotatif avec contrôles manuels pour screenshots
(function() {
    'use strict';

    // Attendre que le DOM soit chargé
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', initCarousel);
    } else {
        initCarousel();
    }

    function initCarousel() {
        const carousel = document.getElementById('screenshotsCarousel3D');
        if (!carousel) {
            console.warn('[Screenshots 3D Carousel] Carousel element not found');
            return;
        }

        const items = carousel.querySelectorAll('.screenshots-carousel-3d__item');
        const totalItems = items.length;

        if (totalItems === 0) {
            console.warn('[Screenshots 3D Carousel] No items found');
            return;
        }

        // Utiliser l'angle de rotation absolu au lieu d'un index
        const angleStep = 360 / totalItems;
        let currentRotation = 0;

        // Fonction pour mettre à jour la rotation du carrousel
        function updateCarousel() {
            carousel.style.transform = `translateZ(-35vw) rotateY(${currentRotation}deg)`;
            console.log(`[Screenshots 3D Carousel] Rotation: ${currentRotation}deg`);
        }

        // Navigation vers le slide suivant
        function nextSlide() {
            currentRotation -= angleStep;
            updateCarousel();
        }

        // Navigation vers le slide précédent
        function prevSlide() {
            currentRotation += angleStep;
            updateCarousel();
        }

        // Gestion des clics sur les items (PC)
        items.forEach((item) => {
            item.addEventListener('click', function(e) {
                e.stopPropagation();

                // Calculer la position relative de l'item par rapport au centre
                const rect = item.getBoundingClientRect();
                const centerX = window.innerWidth / 2;
                const itemCenterX = rect.left + rect.width / 2;

                // Si l'item est à droite du centre, aller au suivant
                // Si l'item est à gauche du centre, aller au précédent
                if (itemCenterX > centerX) {
                    nextSlide();
                } else if (itemCenterX < centerX) {
                    prevSlide();
                }
                // Si c'est l'item central, ne rien faire
            });
        });

        // Gestion du swipe tactile (Mobile)
        let touchStartX = 0;
        let touchEndX = 0;
        const swipeThreshold = 50;

        carousel.addEventListener('touchstart', function(e) {
            e.stopPropagation(); // Éviter les conflits avec d'autres carrousels
            touchStartX = e.touches[0].clientX;
        }, { passive: true });

        carousel.addEventListener('touchend', function(e) {
            e.stopPropagation(); // Éviter les conflits avec d'autres carrousels
            touchEndX = e.changedTouches[0].clientX;
            handleSwipe();
        }, { passive: true });

        function handleSwipe() {
            const diff = touchStartX - touchEndX;

            if (Math.abs(diff) > swipeThreshold) {
                if (diff > 0) {
                    // Swipe left -> next
                    nextSlide();
                } else {
                    // Swipe right -> prev
                    prevSlide();
                }
            }
        }

        // Initialiser le carrousel
        updateCarousel();

        console.log('[Screenshots 3D Carousel] Initialized with manual controls');
    }
})();
