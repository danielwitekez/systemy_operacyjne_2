#include <linux/module.h>
#include <linux/rbtree.h>
#include <linux/slab.h>

struct przyklad_struktura {
    int dane;
    struct rb_node wezel;
};

static struct rb_root korzen = RB_ROOT;

static struct przyklad_struktura *znajdz_wezel(struct rb_root *korzen, int numer) {
    struct rb_node *wezel = korzen->rb_node;

    while (wezel) {
        struct przyklad_struktura *aktualne_dane = rb_entry(wezel, struct przyklad_struktura, wezel);

        if (numer < aktualne_dane->dane)
            wezel = wezel->rb_left;
        else if (numer > aktualne_dane->dane)
            wezel = wezel->rb_right;
        else
            return aktualne_dane;
    }
    return NULL;
}

static bool wstaw_wezel(struct rb_root *korzen, struct przyklad_struktura *wezel) {
    struct rb_node **nowy_wezel = &(korzen->rb_node), *rodzic = NULL;

    while (*nowy_wezel) {
        struct przyklad_struktura *ten = rb_entry(*nowy_wezel, struct przyklad_struktura, wezel);
        rodzic = *nowy_wezel;
        if (wezel->dane < ten->dane)
            nowy_wezel = &((*nowy_wezel)->rb_left);
        else if (wezel->dane > ten->dane)
            nowy_wezel = &((*nowy_wezel)->rb_right);
        else
            return false;
    }

    rb_link_node(&wezel->wezel, rodzic, nowy_wezel);
    rb_insert_color(&wezel->wezel, korzen);
    return true;
}

static void zastap_wezel(struct rb_root *korzen, struct przyklad_struktura *stary, struct przyklad_struktura *nowy) {
    rb_replace_node(&stary->wezel, &nowy->wezel, korzen);
    nowy->wezel.rb_left = stary->wezel.rb_left;
    nowy->wezel.rb_right = stary->wezel.rb_right;
}

static int __init rbtreemod_init(void) {
    int i;
    struct przyklad_struktura *wezel = NULL;

    for (i = 0; i < 10; ++i) {
        wezel = (struct przyklad_struktura *)kmalloc(sizeof(struct przyklad_struktura), GFP_KERNEL);
        if (!IS_ERR(wezel)) {
            wezel->dane = i * 2;
            if (!wstaw_wezel(&korzen, wezel))
                pr_alert("Blad podczas wstawiania nowego wezla!\n");
        } else
            pr_alert("Blad alokacji pamieci dla nowego wezla: %ld\n", PTR_ERR(wezel));
    }

    pr_info("Drzewo czerwono-czarne z wezlami w porzadku rosnacym:\n");
    for (i = 0; i < 10; ++i) {
        wezel = znajdz_wezel(&korzen, i * 2);
        if (wezel)
            pr_info("Wartosc wezla: %d\n", wezel->dane);
        else
            pr_alert("Blad podczas pobierania wezla z drzewa czerwono-czarnego!\n");
    }

    pr_info("Drzewo czerwono-czarne z wezlami w porzadku malejacym:\n");
    for (i = 9; i >= 0; --i) {
        wezel = znajdz_wezel(&korzen, i * 2);
        if (wezel)
            pr_info("Wartosc wezla: %d\n", wezel->dane);
        else
            pr_alert("Blad podczas pobierania wezla z drzewa czerwono-czarnego!\n");
    }

    // Test funkcji zastap_wezel
    struct przyklad_struktura *nowy_wezel = (struct przyklad_struktura *)kmalloc(sizeof(struct przyklad_struktura), GFP_KERNEL);
    if (!IS_ERR(nowy_wezel)) {
        nowy_wezel->dane = 100;
        struct przyklad_struktura *stary_wezel = znajdz_wezel(&korzen, 6);
        if (stary_wezel) {
            pr_info("Zastepowanie wezla o wartosci %d wezlem o wartosci %d\n", stary_wezel->dane, nowy_wezel->dane);
            zastap_wezel(&korzen, stary_wezel, nowy_wezel);
            kfree(stary_wezel);
        } else
            pr_alert("Nie znaleziono wezla do zastapienia!\n");
    } else
        pr_alert("Blad alokacji pamieci dla nowego wezla do zastapienia!\n");

    return 0;
}

static void __exit rbtreemod_exit(void) {
    int i;
    struct przyklad_struktura *wezel = NULL;

    for (i = 0; i < 10; ++i) {
        wezel = znajdz_wezel(&korzen, i * 2);
        if (wezel) {
            rb_erase(&wezel->wezel, &korzen);
            kfree(wezel);
        } else
            pr_alert("Blad podczas pobierania wezla z drzewa czerwono-czarnego!\n");
    }
}

module_init(rbtreemod_init);
module_exit(rbtreemod_exit);

MODULE_LICENSE("GPL");
