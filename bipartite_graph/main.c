#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <float.h>
#include <stdbool.h>
#define MAX_USERS 1000
#define MAX_ITEMS 2000
#define MAX_EDGES 100000

#define MAX_GENRES 24  

//filmlerin genrelari
char *genre_columns[MAX_GENRES] = {
    "unknown", "Action", "Adventure", "Animation", "Children's", "Comedy", "Crime", "Documentary", "Drama", "Fantasy",
    "Film-Noir", "Horror", "Musical", "Mystery", "Romance", "Sci-Fi", "Thriller", "War", "Western"
};


//gerekli structlar, userlar filmler edgeler ve weighted path algoritmasi icin queue
typedef struct Edge {
    int userID;
    int itemID;
    int rating;
} Edge;

typedef struct User {
    int userID;
    int *watchedItems;
    int watchedCount;
} User;

typedef struct Item {
    int itemID;
    int *watchedByUsers;
    int watchedByCount;
    int genres[MAX_GENRES];
} Item;

typedef struct Queue {
    int *users;
    int *depths;
    int front;
    int rear;
} Queue;

struct Recommendation {
    int itemID;
    double score;
};

//edge, user ve structlarin tutulacagi arrayler. Veri boyutu belirli oldugu icin array kullanmayi tercih ettim. aradigim veriye ulasmak daha kolay.
Edge edges[MAX_EDGES];
User users[MAX_USERS];
Item items[MAX_ITEMS];
int edgeCount = 0, userCount = 0, itemCount = 0;

//u.data dosyasindan verileri okuyuo user, edge ve item structlarini olusturan fonksiyon.
void readData(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    int userID, itemID, rating, timestamp;
    while (fscanf(file, "%d\t%d\t%d\t%d", &userID, &itemID, &rating, &timestamp) == 4) {
        edges[edgeCount++] = (Edge){userID, itemID, rating};

        if (userID >= userCount) userCount = userID + 1;
        if (itemID >= itemCount) itemCount = itemID + 1;
    }

    fclose(file);
    printf("Users: %d, Items: %d\n", userCount, itemCount);
}

//structlari baslatan ve arraylere dizen fonksiyon
void initialize() {
    for (int i = 0; i < userCount; i++) {
        users[i].userID = i;
        users[i].watchedItems = (int *)malloc(MAX_ITEMS * sizeof(int));  
        users[i].watchedCount = 0;
    }

    for (int i = 0; i < itemCount; i++) {
        items[i].itemID = i;
        items[i].watchedByUsers = (int *)malloc(MAX_USERS * sizeof(int));  
        items[i].watchedByCount = 0;
    }

    
    for (int i = 0; i < edgeCount; i++) {
        Edge e = edges[i];
        if (e.userID < userCount && e.itemID < itemCount) {
            users[e.userID].watchedItems[users[e.userID].watchedCount++] = e.itemID;
            items[e.itemID].watchedByUsers[items[e.itemID].watchedByCount++] = e.userID;
        }
    }
}
//verilen userin verilen filmi izleyip izlemedigini kontrol eden fonksiyon
int notwatched(int user_id, int item_id){
    int k=1;
    for(int i=0; i<users[user_id].watchedCount;i++){
        if(item_id==users[user_id].watchedItems[i]){
            k=0;
        }
    }
    return k;
}



void recommendRandom(int userID) {
    printf("User %d icin rastgele oneri:\n", userID);
    int notWatched[MAX_ITEMS], count = 0;

    //user tarafindan izlenmemis filmler listesi olusturuluyor
    for (int i = 0; i < itemCount; i++) {
        if (notwatched(userID, i)) {
            notWatched[count++] = i;
        }
    }
    //eger bu liste doluysa icinden 5 tane random oneri yap
    if (count > 0) {
        srand(time(0)); 
        int recommendedItems[5] = {-1, -1, -1, -1, -1}; 
        int numRecommendations = count < 5 ? count : 5; //izlenmemis filmler 5ten azsa o kadar oneri yapilacak, az degilse 5 tane

        for (int i = 0; i < numRecommendations; i++) {
            int randomIndex;
            do {
                randomIndex = rand() % count; 
            } while (recommendedItems[0] == notWatched[randomIndex] || 
                     recommendedItems[1] == notWatched[randomIndex] || 
                     recommendedItems[2] == notWatched[randomIndex] || 
                     recommendedItems[3] == notWatched[randomIndex] || 
                     recommendedItems[4] == notWatched[randomIndex]);

            recommendedItems[i] = notWatched[randomIndex]; 
            printf("Item %d\n", recommendedItems[i]);
        }
    } else {
        printf("No items to recommend.\n");
    }
    printf("\n\n");
}

//derecesi en buyuk olan 5 itemi yani en cok izlenen 5 filmi yazdiran fonksiyon 
void recommendMostWatched(int userID) {
    printf("User %d icin en cok izlenen filmler uzerinden oneri:\n", userID);

    int notWatched[MAX_ITEMS], count = 0;

    //izlenmemis filmler listesi olusturuluyor.
    for (int i = 0; i < itemCount; i++) {
        if (notwatched(userID, i)) {
            notWatched[count++] = i;
        }
    }

    if (count == 0) {
        printf("No items to recommend.\n");
        return;
    }

    
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (items[notWatched[j]].watchedByCount < items[notWatched[j + 1]].watchedByCount) { //bubble sort ile izlenmemis 
                                                                                    //filmler siralaniyor ve izlenmemis ilk 5 film seciliyor
                
                int temp = notWatched[j];
                notWatched[j] = notWatched[j + 1];
                notWatched[j + 1] = temp;
            }
        }
    }

    
    int numRecommendations = count < 5 ? count : 5; 
    for (int i = 0; i < numRecommendations; i++) {
        int itemID = notWatched[i];
        printf("Item %d %d kisi tarafindan izlendi\n", itemID, items[itemID].watchedByCount);
    }
    printf("\n\n");
}

//izlenen ortak filmlere gore benzer user bularak onun izledigi en yuksek puanli filmleri onerme
void recommendSimilarUserFav(int userID) {
    printf("User %d icin en benzedigi userin en sevdigi filmler uzerinden oneri:\n", userID);

    int maxOverlap = 0, similarUser = -1;

    
    for (int i = 0; i < userCount; i++) {
        if (i == userID) continue;
        //izlnen ortak filmleri bul
        int overlap = 0;
        for (int j = 0; j < users[userID].watchedCount; j++) {
            for (int k = 0; k < users[i].watchedCount; k++) {
                if (users[userID].watchedItems[j] == users[i].watchedItems[k]) {
                    overlap++;
                }
            }
        }
        //en fazla ortakliga sahip kullaniciyi guncelle
        if (overlap > maxOverlap) {
            maxOverlap = overlap;
            similarUser = i;
        }
    }

    if (similarUser == -1) {
        printf("No similar user found.\n");
        return;
    }

    printf("En benzer user: %d ortak izlenen film sayisi: %d\n", similarUser, maxOverlap);

    
    int notWatched[MAX_ITEMS], count = 0;
    //benzer kullanicinin izledigi filmler arasindan userin izlemedigi filmler listesi olustur
    for (int i = 0; i < users[similarUser].watchedCount; i++) {
        int movie = users[similarUser].watchedItems[i];
        if (notwatched(userID, movie)) {
            notWatched[count++] = movie;
        }
    }

    if (count == 0) {
        printf("No movies to recommend from similar user.\n");
        return;
    }

    //filmlerin id ve ortalama ratinglerini tutan struct
    struct Movie {
        int movieID;
        int averageRating;
    } movies[MAX_ITEMS];

    int movieCount = 0;
    //filmlerin ortalama ratinglerini bul
    for (int i = 0; i < count; i++) {
        int currentMovie = notWatched[i];
        int totalRating = 0, ratingCount = 0;

        for (int j = 0; j < edgeCount; j++) {
            if (edges[j].itemID == currentMovie && edges[j].userID == similarUser) {
                totalRating += edges[j].rating;
                ratingCount++;
            }
        }

        if (ratingCount > 0) {
            movies[movieCount].movieID = currentMovie;
            movies[movieCount].averageRating = totalRating / ratingCount;
            movieCount++;
        }
    }

    if (movieCount == 0) {
        printf("No suitable movies found for recommendation.\n");
        return;
    }

    //filmleri ortalama ratinge gore bubble sort ile sirala
    for (int i = 0; i < movieCount - 1; i++) {
        for (int j = 0; j < movieCount - i - 1; j++) {
            if (movies[j].averageRating < movies[j + 1].averageRating) {
                struct Movie temp = movies[j];
                movies[j] = movies[j + 1];
                movies[j + 1] = temp;
            }
        }
    }

    //film sayisi 5ten azsa o kadar, degilse 5 oneri yap
    int recommendations = movieCount < 5 ? movieCount : 5;
    for (int i = 0; i < recommendations; i++) {
        printf("%d id'li film. ortalama rating: %d\n", movies[i].movieID, movies[i].averageRating);
    }
    printf("\n\n");
}
void recommendWeightedshortestPath(int userA, int maxDepth) {
    printf("Weighted path recommendations for user %d with depth limit %d:\n", userA, maxDepth);

    struct Recommendation {          //onerileri tutmak icin struct
        int itemID;
        double score;
    } recommendations[5];

    for (int i = 0; i < 5; i++) {
        recommendations[i].itemID = -1;             //oneri listesi -1 ile dolduruluyor
        recommendations[i].score = FLT_MAX;        //Skorlar sonsuza esitleniyor
    }

    int visited[MAX_USERS + MAX_ITEMS] = {0};      
    int queue[MAX_USERS + MAX_ITEMS], front = 0, rear = 0;  //BFS icin queue yapisi
    double dist[MAX_USERS + MAX_ITEMS];                     //uzaklik tutacak array

    for (int i = 0; i < MAX_USERS + MAX_ITEMS; i++) {
        dist[i] = FLT_MAX;  // Baslangicta tum mesafeleri sonsuz olarak ayarla
    }

    queue[rear++] = userA;                      //useri queueye ekle
    visited[userA] = 1;                         
    dist[userA] = 0;

    while (front < rear) {
        int current = queue[front++];

        if (current < userCount) { // User dugumu ise
            for (int i = 0; i < users[current].watchedCount; i++) {
                int nextItem = users[current].watchedItems[i];       //Userin izledigi filmler arasindan ziyaret edilmemis olanlari ziyaret et
                if (!visited[nextItem + MAX_USERS]) {
                    visited[nextItem + MAX_USERS] = 1;

                    // Baglanti kontrolu ve agirlik hesaplama
                    int rating = -1;
                    for (int j = 0; j < edgeCount; j++) {
                        if (edges[j].userID == current && edges[j].itemID == nextItem) {
                            rating = edges[j].rating;              //ziyaret edilen itemin ratingini al
                            break;
                        }
                    }

                    if (rating == -1) continue;        //rating bulunamadiysa devam et

                    double weight = 1.0 / rating;      //agirligi 1/rating olarak tanimlayip en kucuk agirlikli yolu buluyorum.
                    dist[nextItem + MAX_USERS] = dist[current] + weight;   //bu nodea gelene kadar aldigim yolu, 
                                                                        //yani yol uzerindeki weight toplamini diziye kaydediyorum
                    queue[rear++] = nextItem + MAX_USERS;               //next itemi queueya ekliyorum

                    if (dist[nextItem + MAX_USERS] <= maxDepth && notwatched(userA, nextItem)) {//next item izlenmediyse ve max depth asilmadiysa
                        double score = dist[nextItem + MAX_USERS];      //skoru yol agirligi olarak guncelliyorum.
                        for (int k = 0; k < 5; k++) {
                            if (score < recommendations[k].score) {     //skoru oneri listesinde dogru yere yerlestiriyorum
                                for (int l = 4; l > k; l--) {           
                                    recommendations[l] = recommendations[l - 1];  //insertion sort benzeri bir yontem kullaniyorum
                                }
                                recommendations[k].itemID = nextItem;
                                recommendations[k].score = score;
                                break;
                            }
                        }
                    }
                }
            }
        } else { // Film dugumu ise
            int itemID = current - MAX_USERS;   //item id'sini cekiyorum

            for (int i = 0; i < items[itemID].watchedByCount; i++) {
                int nextUser = items[itemID].watchedByUsers[i];     //sirayla filmi izleyen userlara gidecegiz
                if (!visited[nextUser]) {
                    visited[nextUser] = 1;                         //ziyaret edilmediyse ziyaret et

                    // Baglanti kontrolu ve agirlik hesaplama
                    int rating = -1;
                    for (int j = 0; j < edgeCount; j++) {
                        if (edges[j].userID == nextUser && edges[j].itemID == itemID) {
                            rating = edges[j].rating;                //ratingi cekiyoruz
                            break;
                        }
                    }

                    if (rating == -1) continue;      //ratingi bulamadiysan devam et

                    double weight = 1.0 / rating;        //agirlik hesaplaniyor
                    dist[nextUser] = dist[current] + weight;        //pathin toplam agirligi guncelleniyor

                    queue[rear++] = nextUser;                       //next user queueya eklendi 

                    if (dist[nextUser] <= maxDepth && notwatched(userA, itemID)) {  //userA filmi izlemediyse ve max depth asilmadiysa
                        double score = dist[nextUser];                             //Skoru guncelle
                        for (int k = 0; k < 5; k++) {                              //Skor listeye eklenecekse uygun konuma insertion sort benzeri
                            if (score < recommendations[k].score) {                 //bir sekilde ekleniyor
                                for (int l = 4; l > k; l--) {
                                    recommendations[l] = recommendations[l - 1];
                                }
                                recommendations[k].itemID = itemID;
                                recommendations[k].score = score;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    int recommendationCount = 0;
    for (int i = 0; i < 5; i++) {                           //oneriler yazdiriliyor
        if (recommendations[i].itemID != -1) {
            printf("Recommended item: %d with score %f\n", recommendations[i].itemID, recommendations[i].score);
            recommendationCount++;
        }
    }

    if (recommendationCount == 0) {
        printf("No items to recommend.\n");
    }
    printf("\n\n");
}




void readGenre(const char *filename) {      //u.item dosyasindan genrelar okunuyor.
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    char line[1024];
    int itemCount = 0;

    
    while (fgets(line, sizeof(line), file)) {
        if (itemCount >= MAX_ITEMS) {
            printf("Maximum item count exceeded, stopping...\n");
            break;  
        }
        Item *currentItem = &items[itemCount];

        
        char *token = strtok(line, "|");      //binary bicimde aralarinda | ile belirtilen genre turleri okunup kaydediliyor
        currentItem->itemID = atoi(token);  

        
        token = strtok(NULL, "|");    //genre stringlerini ayraclarla bol

        
        for (int i = 0; i < MAX_GENRES; i++) {
            token = strtok(NULL, "|");
            if (token == NULL) {
                currentItem->genres[i] = 0;   //itemlarin genrelarini kaydet
            } else {
                currentItem->genres[i] = atoi(token);  
            }
        }

        itemCount++;
    }

    fclose(file);
}

void printItemGenres(int itemID) {      //verilen itemin genreini yazdiran fonksiyon
    Item *currentItem = &items[itemID];

    printf("Item %d:\nGenres: ", currentItem->itemID);

    
    for (int i = 0; i < MAX_GENRES; i++) {
        if (currentItem->genres[i] == 1) {
            printf("%s, ", genre_columns[i]);
        }
    }

    printf("\n");
}

int getRating(int user_id, int item_id) {     //verilen userin verilen itema verdigi ratingi veren fonksiyon
    for (int i = 0; i < edgeCount; i++) {
        if (edges[i].userID == user_id && edges[i].itemID == item_id) {
            return edges[i].rating;
        }
    }
    return 0;  // Eger kullanici bu filme puan vermediyse 0 dondur
}




int findFavGenre(int user_id) {  // Verilen kullanicinin favori turunu bulan fonksiyon
    int fav_genre = 0;
    int max_genre_score = 0;  // Turlerin toplam puanini tutmak icin
    int genre_score = 0;
    User user = users[user_id];

    for (int k = 1; k < MAX_GENRES; k++) {   // Kullanicinin izledigi tum filmlere bakiliyor.
        genre_score = 0;
        for (int i = 0; i < user.watchedCount; i++) {
            int item_id = user.watchedItems[i];
            genre_score += items[item_id].genres[k] * getRating(user_id, item_id);  //film bir genrea ait ise ona verilen rating genre_scorea ekleniyor.
        }                                                                           //film o genredan degilse items[item_id].genres[k] 0 olacagi icin bir sey eklenmeyecek
        if (genre_score > max_genre_score) {
            max_genre_score = genre_score;                                  //maksimum skorlu genrei dondur
            fav_genre = k;
        }
    }
    return fav_genre;
}

int* item_by_genre(int genre){                     //verilen genrea sahip tum itemleri donduren fonksiyon
    
    int* genred_items=malloc(sizeof(int)*MAX_ITEMS);
    int count=1;
    for(int i=1;i<itemCount;i++){
        for(int k=0; k<MAX_GENRES;k++){
            if(items[i].genres[k]==genre){
                genred_items[count]=i;
                count++;


            }
        }
    }
    genred_items[0]=count;
    return genred_items;
}
double mean_rating(int item_id) {     //verilen itemin ortalama ratingini hesaplayan fonksiyon
    double rating_sum = 0.0;
    double count = 0.0;

    for (int i = 0; i < edgeCount; i++) { //tum interaksiyonlara bailip ortalama rating hesaplaniyor
        if (edges[i].itemID == item_id) {
            rating_sum += edges[i].rating;
            count++;
        }
    }

    if (count == 0) {
        return 0;  
    }
    


    return rating_sum / count;  
}


void recommendHighestRated_from_array(int* array, int size) {  //Verilen arrayde en yuksek ortalamali filmleri dondur
    int best_items[5] = {-1, -1, -1, -1, -1};
    double best_ratings[5] = {0, 0, 0, 0, 0};

    for (int i = 0; i < size; i++) {
        double rating = mean_rating(array[i]); //her elemanin ortalama ratingine bak

        
        for (int j = 0; j < 5; j++) {
            if (rating > best_ratings[j]) {
                //item uygun konuma yerlestiriliyor
                for (int k = 4; k > j; k--) {
                    best_ratings[k] = best_ratings[k - 1];
                    best_items[k] = best_items[k - 1];
                }
                
                best_ratings[j] = rating;
                best_items[j] = array[i];
                break;
            }
        }
    }

    
    
    for (int i = 0; i < 5; i++) {
        if (best_items[i] != -1) {
            printf("Item ID: %d, Rating: %f\n", best_items[i], best_ratings[i]);
        }
    }
}


void recommendFromFavGenre(int user_id){      
    int fav_genre=findFavGenre(user_id);
    printf("%d idli user icin genre temelli oneri. Userin en sevdigi genre: %s\n", user_id,genre_columns[fav_genre]);
    recommendHighestRated_from_array(item_by_genre(fav_genre), item_by_genre(fav_genre)[0]);
    //onceki fonksiyonlar kullanilarak fav genre bulunuyor, ve o genrein en iyi 5 filmi oneriliyor.
    return;

}






int main() {
    const char *filename = "u.data";  

    const char *genre = "u.item";  
    readGenre(genre);

    
    readData(filename);
    initialize();
    int a;
    printf("oneri yapilacak useri id'sini girin.");

    scanf("%d", &a);
    if(a>944 || a<1){
        printf("Gecersiz user. 1 ve 945 araliginda user secin");
        exit(1);
    }
    
    recommendRandom(a);
    recommendMostWatched(a);
    recommendSimilarUserFav(a);
    recommendWeightedshortestPath(a,10);
    recommendFromFavGenre(a);

    
    for (int i = 0; i < userCount; i++) {
        free(users[i].watchedItems);
    }
    for (int i = 0; i < itemCount; i++) {
        free(items[i].watchedByUsers);
    }

    return 0;
}
