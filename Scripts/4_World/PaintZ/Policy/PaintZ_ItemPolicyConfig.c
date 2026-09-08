class PaintZ_ItemPolicyRule
{
    string _example;
    string action;
    string type;
    string class_pattern;
    string inherits;
};

class PaintZ_ItemPolicyConfig
{
    string _description;
    string _state_help;
    string _reload_help;
    string _default_action_help;
    ref array<string> _rule_help;
    ref array<string> _examples;

    int version;
    int reload_seconds = -999999;
    string default_action;
    ref array<ref PaintZ_ItemPolicyRule> rules;

    void PaintZ_ItemPolicyConfig()
    {
        _rule_help = new array<string>;
        _examples = new array<string>;
        rules = new array<ref PaintZ_ItemPolicyRule>;
    }
};
