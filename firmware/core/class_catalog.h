#ifndef PD_CLASS_CATALOG_H
#define PD_CLASS_CATALOG_H
#define PD_OUTFITS 13
static inline const char *pd_outfit_name(unsigned id){
    static const char *const names[]={"Originale","Barbaro","Bardo","Chierico","Druido","Guerriero","Monaco","Paladino","Ranger","Ladro","Stregone","Warlock","Mago"};
    return id<PD_OUTFITS?names[id]:"Classe non valida";
}
#endif
