#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITEMS 20
#define NAME_SIZE 80

//ESTRUTURAS DO CARRINHO
typedef struct {
    char name[NAME_SIZE];
    double unit_price;
    int quantity;
} CartItem;

typedef struct {
    CartItem items[MAX_ITEMS];
    int item_count;
} Cart;

//PADRÃO STRATEGY - Algoritmos de Frete

typedef struct {
    const char *name;
    double (*calculate)(double subtotal, double weight);
} ShippingStrategy;

static double calculate_standard_shipping(double subtotal, double weight) {
    double price = 12.0 + weight * 1.50;
    return subtotal >= 200.0 ? 0.0 : price;
}

static double calculate_express_shipping(double subtotal, double weight) {
    (void)subtotal;
    return 25.0 + weight * 2.50;
}

static double calculate_pickup_shipping(double subtotal, double weight) {
    (void)subtotal;
    (void)weight;
    return 0.0;
}

// Definição das estratégias disponíveis
static const ShippingStrategy STRATEGY_STANDARD = {"Standard", calculate_standard_shipping};
static const ShippingStrategy STRATEGY_EXPRESS  = {"Expresso", calculate_express_shipping};
static const ShippingStrategy STRATEGY_PICKUP   = {"Retirada na loja", calculate_pickup_shipping};

typedef enum {
    SHIPPING_STANDARD = 1,
    SHIPPING_EXPRESS  = 2,
    SHIPPING_PICKUP   = 3
} ShippingType;

/*PADRÃO FACTORY (Criação/Seleção)
A Factory encapsula a regra de qual objeto Strategy retornar com base no tipo solicitado */
const ShippingStrategy* shipping_strategy_factory(ShippingType type) {
    switch (type) {
        case SHIPPING_STANDARD:
            return &STRATEGY_STANDARD;
        case SHIPPING_EXPRESS:
            return &STRATEGY_EXPRESS;
        case SHIPPING_PICKUP:
            return &STRATEGY_PICKUP;
        default:
            return NULL;
    }
}

//FUNÇÕES AUXILIARES DO CARRINHO
static void cart_init(Cart *cart) {
    cart->item_count = 0;
}

static int cart_add_item(Cart *cart, const char *name, double unit_price, int quantity) {
    if (cart->item_count >= MAX_ITEMS || unit_price < 0.0 || quantity <= 0) {
        return 0;
    }

    CartItem *item = &cart->items[cart->item_count];
    snprintf(item->name, NAME_SIZE, "%s", name);
    item->unit_price = unit_price;
    item->quantity = quantity;
    cart->item_count++;
    return 1;
}

static double cart_subtotal(const Cart *cart) {
    double subtotal = 0.0;
    for (int i = 0; i < cart->item_count; i++) {
        subtotal += cart->items[i].unit_price * cart->items[i].quantity;
    }
    return subtotal;
}

static double cart_weight(const Cart *cart) {
    double weight = 0.0;
    for (int i = 0; i < cart->item_count; i++) {
        weight += 0.5 * cart->items[i].quantity;
    }
    return weight;
}

static double coupon_discount(const char *coupon, double subtotal) {
    if (strcmp(coupon, "DESC10") == 0) {
        return subtotal * 0.10;
    }
    if (strcmp(coupon, "DESC20") == 0 && subtotal >= 300.0) {
        return subtotal * 0.20;
    }
    return 0.0;
}

static void print_cart(const Cart *cart) {
    printf("\nItens do carrinho:\n");
    for (int i = 0; i < cart->item_count; i++) {
        const CartItem *item = &cart->items[i];
        printf("%d. %s - %d x R$ %.2f = R$ %.2f\n", i + 1, item->name,
               item->quantity, item->unit_price,
               item->unit_price * item->quantity);
    }
}

static void print_shipping_options(void) {
    printf("\nEscolha o frete:\n");
    printf("1. Standard\n");
    printf("2. Expresso\n");
    printf("3. Retirada na loja\n");
}

//FLUXO PRINCIPAL (CLIENTE)
int main(void) {
    Cart cart;
    int item_count;

    cart_init(&cart);

    printf("Modulo de Carrinho de Compras\n");
    printf("Quantidade de produtos: ");
    if (scanf("%d", &item_count) != 1 || item_count <= 0 || item_count > MAX_ITEMS) {
        fprintf(stderr, "Quantidade invalida.\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < item_count; i++) {
        char name[NAME_SIZE];
        double unit_price;
        int quantity;

        printf("\nProduto %d\n", i + 1);
        printf("Nome: ");
        scanf(" %79[^\n]", name);
        printf("Preco unitario: ");
        if (scanf("%lf", &unit_price) != 1) {
            fprintf(stderr, "Preco invalido.\n");
            return EXIT_FAILURE;
        }
        printf("Quantidade: ");
        if (scanf("%d", &quantity) != 1) {
            fprintf(stderr, "Quantidade invalida.\n");
            return EXIT_FAILURE;
        }

        if (!cart_add_item(&cart, name, unit_price, quantity)) {
            fprintf(stderr, "Nao foi possivel adicionar o produto.\n");
            return EXIT_FAILURE;
        }
    }

    int shipping_choice;
    char coupon[NAME_SIZE];

    print_shipping_options();
    printf("Opcao: ");
    if (scanf("%d", &shipping_choice) != 1) {
        fprintf(stderr, "Opcao invalida.\n");
        return EXIT_FAILURE;
    }

    /* USO DO FACTORY*/
    const ShippingStrategy *shipping_strategy = shipping_strategy_factory((ShippingType)shipping_choice);
    if (shipping_strategy == NULL) {
        fprintf(stderr, "Opcao de frete invalida.\n");
        return EXIT_FAILURE;
    }

    printf("Cupom (DESC10, DESC20 ou NONE): ");
    scanf(" %79s", coupon);

    double subtotal = cart_subtotal(&cart);
    double discount = coupon_discount(coupon, subtotal);
    
    /* USO DO STRATEGY:
     A chamada do cálculo é delegada ao objeto retornado pela Factory. */
    double shipping = shipping_strategy->calculate(subtotal, cart_weight(&cart));
    double total = subtotal - discount + shipping;

    print_cart(&cart);
    printf("\nResumo do pedido:\n");
    printf("Subtotal: R$ %.2f\n", subtotal);
    printf("Cupom: %s\n", coupon);
    printf("Desconto: R$ %.2f\n", discount);
    printf("Frete (%s): R$ %.2f\n", shipping_strategy->name, shipping);
    printf("Total: R$ %.2f\n", total);

    return EXIT_SUCCESS;
}