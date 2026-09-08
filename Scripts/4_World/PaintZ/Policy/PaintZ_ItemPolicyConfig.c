class PaintZ_ItemPolicyRule
{
    string action;
    string type;
    string class_pattern;
    string inherits;
    string inventory_slot;
    string inventory_slot_pattern;
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
