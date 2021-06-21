#include <fstream>
#include <Windows.h>
#include <string>
#include <cctype>
#include <conio.h>

// #define _CRT_SECURE_NO_WARNINGS

// Variables
constexpr auto height = 25u;
constexpr auto width = 80u;

char map[height][width];

constexpr auto obj_max = 20u; 
uint32_t obj_count = 0;

struct Point
{
    int x{}, y{};
};

struct Item
{
    char name[20];
};

struct Object
{
    char name[20]{};
    char type{};
    Point position{};

    char requirement[80]{};   // требование, необходимое условие в виде текста
    char required_item[20]{}; // Название требуемого предмета
};

Object* objects[obj_max]{};

void AddObject( Object* obj)
{
    if (obj_count < obj_max)
        objects[obj_count] = obj;

    obj_count++;
}

void ClearObjects()
{
    for (size_t i = 0; i < obj_count; i++)
    {
        delete objects[i];
        objects[i] = nullptr;
    }     
    obj_count = 0;
}

struct Location
{
    char _map[height][width];
    Point size;
}location{};

struct Player
{
    char name[20]{};
    Point position{}, current_loc{};
    Item inventory[20]{};
}player;

bool IsKeyPressed(char input)
{
    return GetKeyState(input) < 0;
}

// Loading
void LoadMapFromFile(const char* file_name)
{
    std::ifstream ifs;
    ifs.open(file_name);

    if (ifs.is_open())
    {
        uint32_t line = 0;

        while (!ifs.eof())      
            ifs.getline(location._map[line++], width);

        location.size.x = strlen(location._map[0]);
        location.size.y = line;
    }   
    ifs.close();   
}

void LoadObjectsFromFile(const char* file_name)
{
    if (obj_count) ClearObjects();

    std::ifstream fin;
    fin.open(file_name);

    if (fin.is_open())
    {
        while (!fin.eof())
        {
            auto object = new Object();
            char buffer[80];
            
            fin.getline(buffer, 20u);
            sprintf_s(object->name, buffer);

            fin.getline(buffer, 20u);
            object->type = buffer[0];

            fin.getline(buffer, 20u);
            object->position.x = std::stoi(buffer);

            fin.getline(buffer, 20u);
            object->position.y = std::stoi(buffer);

            fin.getline(buffer, 80u);
            sprintf_s(object->requirement, buffer);

            fin.getline(buffer, 20u);
            sprintf_s(object->required_item, buffer);

            AddObject(object);
        }
    }
    fin.close();
}

void PutLocationOnMap()
{
    memcpy(map, location._map, sizeof(location._map));
}

void PutObjectsOnMap()
{
    for (uint32_t i = 0; i < obj_count; i++)
        map[objects[i]->position.y][objects[i]->position.x] = objects[i]->type;
}

Object* GetObjectBy_XY(int x, int y)
{
    for (size_t i = 0; i < obj_count; i++)
        if (objects[i]->position.x == x && objects[i]->position.y == y)
            return objects[i];

    return nullptr;
}

uint32_t GetPlayerItemCount(const char* item_name)
{
    uint32_t counter = 0;

    for (size_t i = 0; i < 20u; i++)   
        if (strcmp(player.inventory[i].name, item_name) == 0)
            counter++;
    
    return counter;
}

void AddItemToInventory(Item item)
{
    for (size_t i = 0; i < 20u; i++)   
        if (player.inventory[i].name[0] == '\0')
        {
            sprintf_s(player.inventory[i].name, item.name);
            return;
        }
}

void RemoveItemsFromInventory(const char* item_name, uint32_t count)
{
    uint32_t counter = GetPlayerItemCount(item_name);

    if (counter >= count)  
        for (size_t i = 0; i < 20u; i++)       
            if (count && strcmp(player.inventory[i].name, item_name) == 0)
            {
                memset(player.inventory[i].name, 0, sizeof(player.inventory[i].name));
                count--;
            }
}

void StartDialogWithObject(Object* obj)
{
    if (!obj) return;

    char answer = ' ';

    do
    {
        system("cls");
        printf("%s\n", obj->name);

        if (obj->type == '/')
        {
            if (GetPlayerItemCount(obj->required_item) < 1)
            {
                printf("\n%s\n", obj->requirement);
                printf("\nPress N to exit");
                answer = tolower(_getch());
            }
            else
            {
                printf("\nEnter the door?\tY / N? ");
                answer = tolower(_getch());

                if (answer == 'y')
                {
                    player.position.x += (obj->position.x - player.position.x) * 2;
                    player.position.y += (obj->position.y - player.position.y) * 2;
                    answer = 'n';
                }
            }       
        }

        if (obj->type == 'N')
        {
            printf("\nNeed a %s? ", obj->required_item);

            if (GetPlayerItemCount("Apple") < 5)
            {
                printf(obj->requirement);
                printf("\nPress N to exit");
                answer = tolower(_getch());
            }
            else
            {              
                printf("\nPress Y/N to choose\n");
                answer = tolower(_getch());

                if (answer == 'y')
                {
                    Item key;
                    sprintf_s(key.name, obj->required_item);

                    RemoveItemsFromInventory("Apple", 5u);

                    if (GetPlayerItemCount(key.name) == 0)
                        AddItemToInventory(key);

                    answer = 'n';
                }               
            }        
        }
        
    } while (answer != 'n');
}

void PlayerLoadLocation()
{
    char c[100];
    sprintf_s(c, "map_%d_%d.txt", player.current_loc.x, player.current_loc.y);
    LoadMapFromFile(c);

    sprintf_s(c, "obj_%d_%d.txt", player.current_loc.x, player.current_loc.y);
    LoadObjectsFromFile(c);
}

void RenderMap()
{
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{});

    for (uint32_t i = 0; i < height; i++)
        printf("%s \n", map[i]);
}

// Player
void InitPlayer(const char* name, Point _current_loc, Point player_pos)
{
    player.current_loc = _current_loc;
    player.position = player_pos;
    sprintf_s(player.name, name);
    memset(player.inventory, 0, sizeof(player.inventory));
}

void PlayerPutOnMap()
{
    map[player.position.y][player.position.x] = 'A';

    // Show player`s inventory
    for (size_t i = 0; i < 20; i++)
    {
        COORD coord{ short(85), short(i) };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

        if (player.inventory[i].name[0] != '\0')       
            printf(player.inventory[i].name);      
    }  
}

void SavePlayer()
{
    std::ofstream fout;
    char file_name[24] {};
    snprintf(file_name, 24, "%s%s", player.name, ".dat");
    fout.open(file_name, std::ios::binary);
    fout << player.name << "\n" << player.position.x << "\n" << player.position.y << "\n" << player.current_loc.x << "\n" << player.current_loc.y;

    fout.close();
}

void LoadPlayer()
{
    char user_name[20];
    printf("Enter your name: ");
    scanf_s("%s", user_name, 20);

    std::ifstream fin;
    char file_name[24]{};
    snprintf(file_name, 24, "%s%s", user_name, ".dat");
    fin.open(file_name, std::ios::binary);

    if (fin.is_open())
    {
        char buffer[obj_max];

        fin.getline(buffer, obj_max);
        sprintf_s(player.name, buffer);

        fin.getline(buffer, obj_max);
        player.position.x = std::stoi(buffer);

        fin.getline(buffer, obj_max);
        player.position.y = std::stoi(buffer);

        fin.getline(buffer, obj_max);
        player.current_loc.x = std::stoi(buffer);

        fin.getline(buffer, obj_max);
        player.current_loc.y = std::stoi(buffer);
    }        
    else
        InitPlayer(user_name, {0, 0}, { 5, 5 });

    fin.close();
}

void HandleEvents()
{
    Point old_pos{ player.position };

    if(IsKeyPressed('A')) player.position.x--;
    if(IsKeyPressed('D')) player.position.x++;
    if(IsKeyPressed('W')) player.position.y--;
    if(IsKeyPressed('S')) player.position.y++;

    if (player.position.y != old_pos.y || player.position.x != old_pos.x)
    {
        if (map[player.position.y][player.position.x] != ' ')
        {
            if (map[player.position.y][player.position.x] == 'a')
            {
                Item apple;
                sprintf_s(apple.name, "Apple");

                AddItemToInventory(apple);
                Sleep(500);
            }

            Object* obj = GetObjectBy_XY(player.position.x, player.position.y);
            player.position = old_pos;
            StartDialogWithObject(obj);
        }

        // Check out of location bounds
        //  By X
        if (player.position.x > location.size.x - 2)
        {
            player.current_loc.x++;
            PlayerLoadLocation();
            player.position.x = 1;
        }
        if (player.position.x < 1)
        {
            player.current_loc.x--;
            PlayerLoadLocation();
            player.position.x = location.size.x - 2;
        }
        //  By Y
        if (player.position.y < 1)
        {
            player.current_loc.y--;
            PlayerLoadLocation();
            player.position.y = location.size.y - 2;
        }
        if (player.position.y > location.size.y - 2)
        {
            player.current_loc.y++;
            PlayerLoadLocation();
            player.position.y = 2;
        }
    }   
}

int main()
{
    char path[]{ "map_0_0.txt" };
    LoadPlayer();
    PlayerLoadLocation();
    
    do
    {
        HandleEvents();
        PutLocationOnMap();       
        PlayerPutOnMap();
        PutObjectsOnMap();
        RenderMap();
        Sleep(50);
    } while (!IsKeyPressed(VK_ESCAPE));
    
    SavePlayer(); 
    ClearObjects();

    return 0;
}