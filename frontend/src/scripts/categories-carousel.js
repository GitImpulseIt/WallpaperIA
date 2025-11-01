// Carrousel de catégories - JavaScript isolé
// Utilise window.CAROUSEL_CONFIG pour la configuration

(function() {
    'use strict';

    $(document).ready(function () {
        // Configuration depuis window.CAROUSEL_CONFIG (défini par Astro)
        const API_BASE_URL = window.CAROUSEL_CONFIG?.apiBaseUrl || 'https://kazflow.com/wallpaperai/api';
        const THUMBNAILS_PATH = window.CAROUSEL_CONFIG?.thumbnailsPath || 'https://kazflow.com/wallpaperai/api/miniatures/';
        const IMAGE_LOAD_ERROR = window.CAROUSEL_CONFIG?.imageLoadError || 'Error loading full-size image.';

        let allCategories = [];
        let currentPage = 0;
        let totalPages = 0;
        let itemsPerPage = 9; // Par défaut 9 (3×3), sera 6 (2×3) sur mobile

        // Calculer le nombre d'items par page selon la largeur
        function getItemsPerPage() {
            const containerWidth = $('.cat-carousel-container').width();
            // Sur mobile/tablette (< 768px) : 2 colonnes × 3 lignes = 6 items
            // Sur desktop (≥ 768px) : 3 colonnes × 3 lignes = 9 items
            return containerWidth < 768 ? 6 : 9;
        }

        // Créer une carte de catégorie
        function createCategoryCard(category) {
            return `
                <div class="category-card" data-thumbnail="${category.thumbnail}" data-category-name="${category.name}">
                    <div class="category-image">
                        <img src="${THUMBNAILS_PATH}${category.thumbnail}" alt="${category.name}" loading="lazy">
                    </div>
                    <div class="category-info">
                        <h3 class="category-name">${category.name}</h3>
                    </div>
                </div>
            `;
        }

        // Ouvrir l'image en plein écran (version taille réelle)
        function openFullscreen(thumbnailFilename, categoryName) {
            // Afficher le modal avec le loader
            const $modal = $('#fullscreenModal');
            const $loader = $('.fullscreen-loader');
            const $image = $('#fullscreenImage');

            $loader.show();
            $image.hide();
            $modal.addClass('active');
            $('body').css('overflow', 'hidden');

            // Construire l'URL de l'image en taille réelle via /get/{filename}
            const fullImageUrl = `${API_BASE_URL}/get/${thumbnailFilename}`;

            // Précharger l'image avant de l'afficher
            const img = new Image();
            img.onload = function() {
                $loader.hide();
                $image.attr('src', fullImageUrl);
                $image.attr('alt', categoryName);
                $image.show();
            };
            img.onerror = function() {
                console.error(`[Fullscreen] Failed to load image: ${thumbnailFilename}`);
                closeFullscreen();
                alert(IMAGE_LOAD_ERROR);
            };
            img.src = fullImageUrl;
        }

        // Fermer le modal plein écran
        function closeFullscreen() {
            const $modal = $('#fullscreenModal');
            $modal.removeClass('active');

            // Réactiver le scroll du body
            $('body').css('overflow', '');
        }

        // Créer une slide avec 6 ou 9 catégories selon la taille d'écran
        function createSlide(categories, startIndex) {
            const endIndex = Math.min(startIndex + itemsPerPage, categories.length);
            const slideCategories = categories.slice(startIndex, endIndex);

            let cardsHTML = '';
            slideCategories.forEach(category => {
                cardsHTML += createCategoryCard(category);
            });

            return `
                <div class="cat-carousel-slide">
                    <div class="categories-grid">
                        ${cardsHTML}
                    </div>
                </div>
            `;
        }

        // Mettre à jour le carrousel
        function updateCarousel() {
            const containerWidth = $('.cat-carousel-container').width();
            const offset = -currentPage * containerWidth;

            $('.cat-carousel-slides').css('transform', `translateX(${offset}px)`);

            // Mettre à jour les dots
            $('.cat-carousel-dot').removeClass('active');
            $(`.cat-carousel-dot[data-page="${currentPage}"]`).addClass('active');

            // Mettre à jour l'info de pagination
            $('#currentPage').text(currentPage + 1);
            $('#totalPages').text(totalPages);

            // Activer/désactiver les boutons
            $('#prevCategoriesBtn').prop('disabled', currentPage === 0);
            $('#nextCategoriesBtn').prop('disabled', currentPage === totalPages - 1);
        }

        // Aller à une page spécifique
        function goToPage(pageIndex) {
            if (pageIndex < 0 || pageIndex >= totalPages) return;
            currentPage = pageIndex;
            updateCarousel();
        }

        // Navigation
        function nextPage() {
            if (currentPage < totalPages - 1) {
                goToPage(currentPage + 1);
            }
        }

        function prevPage() {
            if (currentPage > 0) {
                goToPage(currentPage - 1);
            }
        }

        // Appliquer les largeurs du carrousel
        function applyCarouselWidths() {
            const containerWidth = $('.cat-carousel-container').width();
            const $carouselSlides = $('.cat-carousel-slides');
            $carouselSlides.css('width', `${totalPages * containerWidth}px`);

            $('.cat-carousel-slide').each(function (index) {
                $(this).css({
                    'width': `${containerWidth}px`,
                    'min-width': `${containerWidth}px`,
                    'max-width': `${containerWidth}px`
                });
            });

            updateCarousel();
        }

        // Initialiser le carrousel
        function initCarousel(categories) {
            // Afficher le carrousel d'abord pour calculer la largeur
            $('#categoriesCarousel').show();

            // Calculer le nombre d'items par page selon la largeur de l'écran
            itemsPerPage = getItemsPerPage();
            totalPages = Math.ceil(categories.length / itemsPerPage);

            const $carouselSlides = $('#carouselSlides');
            $carouselSlides.empty();

            // Créer les slides
            let slidesHTML = '';
            for (let i = 0; i < totalPages; i++) {
                slidesHTML += createSlide(categories, i * itemsPerPage);
            }
            $carouselSlides.html(slidesHTML);

            // Event listeners pour ouvrir les images en plein écran
            $(document).off('click', '.category-card').on('click', '.category-card', function() {
                const thumbnail = $(this).data('thumbnail');
                const categoryName = $(this).data('category-name');
                openFullscreen(thumbnail, categoryName);
            });

            // Créer les dots
            const $carouselDots = $('#carouselDots');
            $carouselDots.empty();
            for (let i = 0; i < totalPages; i++) {
                const dotClass = i === 0 ? 'cat-carousel-dot active' : 'cat-carousel-dot';
                $carouselDots.append(`<button class="${dotClass}" data-page="${i}"></button>`);
            }

            // Event listeners pour les boutons
            $('#prevCategoriesBtn').off('click').on('click', prevPage);
            $('#nextCategoriesBtn').off('click').on('click', nextPage);

            // Event listeners pour les dots
            $('.cat-carousel-dot').on('click', function () {
                const page = $(this).data('page');
                goToPage(page);
            });

            // Swipe tactile avec jQuery
            let touchStartX = 0;
            let touchEndX = 0;
            let isDragging = false;

            const $container = $('.cat-carousel-container');

            $container.on('touchstart', function (e) {
                e.stopPropagation(); // Éviter les conflits avec d'autres carrousels
                isDragging = true;
                touchStartX = e.originalEvent.touches[0].clientX;
                touchEndX = touchStartX;
            });

            $container.on('touchmove', function (e) {
                if (!isDragging) return;
                e.stopPropagation(); // Éviter les conflits avec d'autres carrousels
                touchEndX = e.originalEvent.touches[0].clientX;

                // Empêcher le scroll si swipe horizontal
                const diff = Math.abs(touchStartX - touchEndX);
                if (diff > 10) {
                    e.preventDefault();
                }
            });

            $container.on('touchend', function (e) {
                if (!isDragging) return;
                e.stopPropagation(); // Éviter les conflits avec d'autres carrousels
                isDragging = false;

                const swipeThreshold = 50;
                const diff = touchStartX - touchEndX;

                if (Math.abs(diff) > swipeThreshold) {
                    if (diff > 0) {
                        nextPage();
                    } else {
                        prevPage();
                    }
                }
            });

            // Gérer le redimensionnement de la fenêtre
            let resizeTimeout;
            $(window).on('resize', function () {
                clearTimeout(resizeTimeout);
                resizeTimeout = setTimeout(function () {
                    // Recalculer le nombre d'items si on change de desktop à mobile ou vice-versa
                    const newItemsPerPage = getItemsPerPage();
                    if (newItemsPerPage !== itemsPerPage) {
                        initCarousel(allCategories);
                    } else {
                        applyCarouselWidths();
                    }
                }, 250);
            });

            // Attendre que les images soient chargées et le DOM rendu
            // Utiliser un délai plus long pour mobile
            requestAnimationFrame(function () {
                requestAnimationFrame(function () {
                    setTimeout(function () {
                        // Vérifier si le container a une largeur valide
                        let containerWidth = $('.cat-carousel-container').width();

                        if (containerWidth === 0) {
                            // Réessayer après un délai supplémentaire
                            setTimeout(function () {
                                applyCarouselWidths();
                            }, 100);
                        } else {
                            applyCarouselWidths();
                        }
                    }, 150);
                });
            });
        }

        // Charger les catégories via AJAX
        function loadCategories() {
            $.ajax({
                url: `${API_BASE_URL}/categories`,
                method: 'GET',
                dataType: 'json',
                success: function (data) {
                    allCategories = data.data || data.categories || [];

                    if (allCategories.length === 0) {
                        throw new Error('No categories found');
                    }

                    // Masquer le loading
                    $('#loadingMessage').hide();

                    // Initialiser le carrousel
                    initCarousel(allCategories);
                },
                error: function (xhr, status, error) {
                    console.error('[Categories] Failed to load:', error);
                    $('#loadingMessage').hide();
                    $('#errorMessage').show();
                }
            });
        }

        // Event listener pour fermer le modal plein écran
        $('#fullscreenModal').on('click', function() {
            closeFullscreen();
        });

        // Empêcher la fermeture si on clique sur l'image elle-même (optionnel)
        // $('#fullscreenImage').on('click', function(e) {
        //     e.stopPropagation();
        // });

        // Fermer avec la touche Échap
        $(document).on('keydown', function(e) {
            if (e.key === 'Escape' || e.keyCode === 27) {
                closeFullscreen();
            }
        });

        // Charger les catégories au démarrage
        loadCategories();
    });
})();
