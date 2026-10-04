using Il2Cpp;
using HarmonyLib;

namespace StumblePriz
{
    [HarmonyPatch(typeof(UIController), "LateUpdate")]
    public static class Patch_TextIntroObjective
    {
        public static bool Prefix(UIController __instance)
        {
            __instance._uilevelIntro._introObjective.text = ".gg/stumblepriz";
            return true;
        }
    }
}