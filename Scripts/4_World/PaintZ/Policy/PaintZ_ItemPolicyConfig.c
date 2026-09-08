class PaintZ_ItemPolicyRule
{
    string action;
    string type;
    string class_pattern;
    ref array<string> class_patterns;
    string inherits;
    ref array<string> inherits_any;
    string inventory_slot;
    ref array<string> inventory_slots;
    string inventory_slot_pattern;
    ref array<string> inventory_slot_patterns;

    void PaintZ_ItemPolicyRule()
    {
        class_patterns = new array<string>;
        inherits_any = new array<string>;
        inventory_slots = new array<string>;
        inventory_slot_patterns = new array<string>;
    }
};

class PaintZ_TargetDomainRule
{
    string type;
    string class_pattern;
    string inventory_slot;
    string inventory_slot_pattern;
};

class PaintZ_ItemPolicyConfig
{
    int version;
    int reload_seconds = -999999;
    string default_action;
    ref array<ref PaintZ_ItemPolicyRule> rules;
    ref array<ref PaintZ_TargetDomainRule> domains;

    void PaintZ_ItemPolicyConfig()
    {
        rules = new array<ref PaintZ_ItemPolicyRule>;
    }
};
