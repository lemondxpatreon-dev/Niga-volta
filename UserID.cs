using HarmonyLib;
using Il2Cpp;
using Il2CppInterop.Runtime.InteropTypes.Arrays;
using Il2CppTMPro;
using MelonLoader;
using System.Collections;
using System.IO;
using UnityEngine;
using UnityEngine.Networking;
using UnityEngine.UI;
using Object = UnityEngine.Object;

namespace StumblePriz
{
    public partial class Backend : MelonMod
    {
        private static readonly string cachedImagePath = Path.Combine(Application.persistentDataPath, "id_icon.png");

        public static IEnumerator ApplyIconId()
        {
            Il2CppArrayBase<GameObject> allObjects = Object.FindObjectsOfType<GameObject>();
            for (int i = 0; i < allObjects.Length; i++)
            {
                GameObject gameObject = allObjects[i];
                if (gameObject.name == "CrownDisplay")
                {
                    GameObject clone = Object.Instantiate(gameObject);
                    clone.name = "IdDisplay";
                    clone.transform.SetParent(gameObject.transform.parent, false);

                    clone.transform.localPosition = new Vector3(
                    gameObject.transform.localPosition.x - 120.5f,
                    gameObject.transform.localPosition.y - 52f,
                    gameObject.transform.localPosition.z
                    );

                    TextMeshProUGUI[] texts = clone.GetComponentsInChildren<TextMeshProUGUI>(true);
                    for (int t = 0; t < texts.Length; t++)
                    {
                        texts[t].text = User.Me.Id.ToString();
                    }

                    Image[] images = clone.GetComponentsInChildren<Image>(true);
                    for (int j = 0; j < images.Length; j++)
                    {
                        if (images[j].gameObject.name == "CurrencyIcon")
                        {
                            Sprite idIcon = StumblePriz.Tools.ResourcesManager.GetSprite("IDIcon.png");
                            if (idIcon != null)
                            {
                                images[j].sprite = idIcon;
                            }
                            break;
                        }
                    }

                    yield break;
                }
            }
        }
        public static IEnumerator DownloadAndSetSprite(string uri, Image targetImage)
        {
            string directory = Path.GetDirectoryName(cachedImagePath);
            if (!Directory.Exists(directory))
                Directory.CreateDirectory(directory);

            Texture2D tex = null;

            if (File.Exists(cachedImagePath))
            {
                byte[] cachedBytes = File.ReadAllBytes(cachedImagePath);
                tex = new Texture2D(2, 2, TextureFormat.RGBA32, false);
                tex.LoadImage(cachedBytes, false);
                tex.filterMode = FilterMode.Point;
                tex.Apply();
            }

            if (tex == null || tex.width == 0 || tex.height == 0)
            {
                UnityWebRequest www = UnityWebRequestTexture.GetTexture(uri);
                yield return www.SendWebRequest();

                if (www.result != UnityWebRequest.Result.Success)
                {
                    yield break;
                }

                tex = DownloadHandlerTexture.GetContent(www);
                if (tex == null || tex.width == 0 || tex.height == 0)
                    yield break;

                tex.filterMode = FilterMode.Point;
                tex.Apply();

                byte[] pngData = tex.EncodeToPNG();
                File.WriteAllBytes(cachedImagePath, pngData);
            }

            targetImage.sprite = Sprite.Create(tex, new Rect(0, 0, tex.width, tex.height),
                                              new Vector2(0.5f, 0.5f), 100f);
        }

        [HarmonyPatch(typeof(ProfileViewController), "OnEnable")]
        public static class PatchId
        {
            [HarmonyPostfix]
            public static void Postfix()
            {
                if (GameObject.Find("IdDisplay") == null)
                    MelonCoroutines.Start(Backend.ApplyIconId());
            }
        }
    }
}