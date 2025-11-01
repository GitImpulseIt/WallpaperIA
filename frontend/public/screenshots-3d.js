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

        const fullscreenModal = document.getElementById('screenshotFullscreenModal');
        const fullscreenImage = document.getElementById('screenshotFullscreenImage');
        const closeBtn = document.getElementById('screenshotCloseBtn');

        // Utiliser l'angle de rotation absolu au lieu d'un index
        const angleStep = 360 / totalItems;
        let currentRotation = 0;

        // Fonction pour mettre à jour la rotation du carrousel
        function updateCarousel() {
            carousel.style.transform = `translateZ(-35vw) rotateY(${currentRotation}deg)`;
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

        // Fonction pour vérifier si un item est au centre
        function isCenterItem(item) {
            const rect = item.getBoundingClientRect();
            const centerX = window.innerWidth / 2;
            const itemCenterX = rect.left + rect.width / 2;
            const threshold = 100; // Tolérance pour considérer un item comme centré
            return Math.abs(itemCenterX - centerX) < threshold;
        }

        // Fonction pour ouvrir le modal plein écran
        function openFullscreen(imageSrc) {
            if (fullscreenImage && fullscreenModal) {
                fullscreenImage.src = imageSrc;
                fullscreenModal.classList.add('active');
                document.body.style.overflow = 'hidden';
            }
        }

        // Fonction pour fermer le modal
        function closeFullscreen() {
            if (fullscreenModal && fullscreenImage) {
                fullscreenModal.classList.remove('active');
                document.body.style.overflow = '';
                fullscreenImage.src = '';
            }
        }

        // Gestion des clics sur les items (PC)
        items.forEach((item) => {
            item.addEventListener('click', function(e) {
                e.stopPropagation();

                // Vérifier si l'item est au centre
                if (isCenterItem(item)) {
                    const img = item.querySelector('img');
                    if (img) {
                        openFullscreen(img.src);
                    }
                    return;
                }

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
            });
        });

        // Gestion du swipe tactile (Mobile)
        let touchStartX = 0;
        let touchStartY = 0;
        let touchEndX = 0;
        let touchEndY = 0;
        let touchStartTime = 0;
        let touchMoved = false;
        const swipeThreshold = 30;
        const tapTimeThreshold = 300; // Temps max pour considérer comme un tap (300ms)

        carousel.addEventListener('touchstart', function(e) {
            touchStartX = e.touches[0].clientX;
            touchStartY = e.touches[0].clientY;
            touchStartTime = Date.now();
            touchMoved = false;
        }, { passive: true });

        carousel.addEventListener('touchmove', function(e) {
            if (touchStartX !== 0) {
                const currentX = e.touches[0].clientX;
                const currentY = e.touches[0].clientY;
                const diffX = Math.abs(currentX - touchStartX);
                const diffY = Math.abs(currentY - touchStartY);

                // Si mouvement > 10px, considérer comme un mouvement
                if (diffX > 10 || diffY > 10) {
                    touchMoved = true;
                }
            }
        }, { passive: true });

        carousel.addEventListener('touchend', function(e) {
            touchEndX = e.changedTouches[0].clientX;
            touchEndY = e.changedTouches[0].clientY;
            const touchDuration = Date.now() - touchStartTime;
            const diffX = touchStartX - touchEndX;
            const diffY = Math.abs(touchStartY - touchEndY);

            // Si c'est un tap rapide sans mouvement significatif
            if (!touchMoved && touchDuration < tapTimeThreshold && Math.abs(diffX) < 10 && diffY < 10) {
                // Trouver l'item touché
                const touch = e.changedTouches[0];
                const element = document.elementFromPoint(touch.clientX, touch.clientY);
                const item = element ? element.closest('.screenshots-carousel-3d__item') : null;

                if (item && isCenterItem(item)) {
                    const img = item.querySelector('img');
                    if (img) {
                        openFullscreen(img.src);
                        return;
                    }
                }
            }

            // Gérer comme un swipe seulement si mouvement suffisant
            if (touchMoved && Math.abs(diffX) > swipeThreshold) {
                handleSwipe();
            }

            // Reset
            touchStartX = 0;
            touchStartY = 0;
            touchMoved = false;
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

        // Fermeture du modal (seulement si les éléments existent)
        if (closeBtn && fullscreenModal && fullscreenImage) {
            closeBtn.addEventListener('click', function(e) {
                e.stopPropagation();
                closeFullscreen();
            });

            fullscreenModal.addEventListener('click', function(e) {
                if (e.target === fullscreenModal) {
                    closeFullscreen();
                }
            });

            // Fermeture avec la touche ESC (seulement pour ce modal spécifique)
            const handleEscape = function(e) {
                if (e.key === 'Escape' && fullscreenModal.classList.contains('active')) {
                    closeFullscreen();
                }
            };
            document.addEventListener('keydown', handleEscape);
        }

        // Initialiser le carrousel
        updateCarousel();
    }
})();
