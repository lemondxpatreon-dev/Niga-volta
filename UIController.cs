using HarmonyLib;
using Il2Cpp;
using Il2CppGimmebreak.Backbone.Core;
using Il2CppTMPro;
using MelonLoader;
using StumblePriz.Tools;
using System;
using System.Collections;
using UnityEngine;
using UnityEngine.UI;
using Object = UnityEngine.Object;

namespace StumblePriz.Modules
{
    internal class UIControllers
    {
        [HarmonyPatch(typeof(UIController), nameof(UIController.LateUpdate))]
        public static class Patch
        {
            public static bool Runing;
            public static PopupController popup;

            [HarmonyPostfix]
            public static void Postfix(UIController __instance)
            {
                if (CustomParty.DisplayDetails)
                {
                    if (UnityEngine.Input.GetKeyDown(KeyCode.F1) && !Runing)
                    {
                        Runing = true;
                        Cursor.visible = true;
                        Cursor.lockState = CursorLockMode.Confined;

                        if (popup == null || !popup.isActiveAndEnabled)
                        {
                            popup = VisualManager.OpenPopup("InfoPopup", "StumblePriz", "<align=center>Do you really want to leave the match?\nIf not click Ok button.");
                        }

                        var button = popup.transform.Find("Button").gameObject;
                        if (button != null)
                        {
                            var localPos = button.transform.localPosition;
                            localPos.x -= 175f;
                            button.transform.localPosition = localPos;

                            var tmp = button.GetComponentInChildren<TextMeshProUGUI>();
                            if (tmp != null)
                            {
                                tmp.text = "Exit";
                            }

                            var btn = button.GetComponent<AnimatedButton>();
                            if (btn != null)
                            {
                                btn.onClick = new UnityEngine.UI.Button.ButtonClickedEvent();
                                btn.onClick.AddListener(new Action(() =>
                                {
                                    __instance._uiLeaveButtons.OnConfirmLeave();
                                    Runing = false;
                                }));
                            }

                            var newButton = Object.Instantiate(button, button.transform.parent);
                            newButton.name = "CancelButton";
                            var newButtonTransform = newButton.transform;
                            var newButtonLocalPos = newButtonTransform.localPosition;
                            newButtonLocalPos.x += 175f;
                            newButtonTransform.localPosition = newButtonLocalPos;

                            var txt = newButton.GetComponent<TextMeshProUGUI>();
                            if (txt != null)
                            {
                                txt.text = "Cancel";
                            }

                            var btn1 = newButton.GetComponent<AnimatedButton>();
                            if (btn1 != null)
                            {
                                btn1.onClick = new UnityEngine.UI.Button.ButtonClickedEvent();
                                btn1.onClick.AddListener(new Action(() =>
                                {
                                    popup.Close();
                                    Cursor.visible = false;
                                    Cursor.lockState = CursorLockMode.None;
                                    Runing = false;
                                    MelonCoroutines.Start(ResetPopup(popup, button, newButton));
                                }));
                            }
                        }
                    }
                }

                string region = PlayerPrefs.GetString("PHOTON_REGION", "EU");
                float pingValue = StumbleManager.GetRoomManager().GetPing() * 0.45f;
                string color = "#00FF00";

                if (pingValue > 10) color = "#40FF00";
                if (pingValue > 20) color = "#80FF00";
                if (pingValue > 30) color = "#C0FF00";
                if (pingValue > 40) color = "#FFFF00";
                if (pingValue > 50) color = "#FFC000";
                if (pingValue > 60) color = "#FF8000";
                if (pingValue > 70) color = "#FF4000";
                if (pingValue > 80) color = "#FF2000";
                if (pingValue > 90) color = "#FF1000";
                if (pingValue > 100) color = "#FF0000";

                __instance._uiPing._pingLabel.text = $"<color=#00FFFF>[{region}]</color> <color=white>Ping</color> <color={color}>{pingValue:0}ms</color>";
            }

            private static IEnumerator ResetPopup(PopupController popupController, GameObject originalButton, GameObject cancelButton)
            {
                if (popupController != null && popupController.isActiveAndEnabled)
                {
                    popupController.Close();
                }

                yield return new WaitForSeconds(0.1f);

                if (originalButton != null)
                {
                    var localPos = originalButton.transform.localPosition;
                    localPos.x += 175f;
                    originalButton.transform.localPosition = localPos;

                    var tmp = originalButton.GetComponentInChildren<TextMeshProUGUI>();
                    if (tmp != null)
                    {
                        tmp.text = "Cancel";
                    }

                    var btn = originalButton.GetComponent<AnimatedButton>();
                    if (btn != null)
                    {
                        btn.onClick = new UnityEngine.UI.Button.ButtonClickedEvent();
                    }
                }

                if (cancelButton != null)
                {
                    Object.Destroy(cancelButton);
                }

                popup = null;
                Runing = false;
            }
        }
    }
}