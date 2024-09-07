# MMASPlugin

MMASPlugin is designed to provide a streamlined setup for a complete locomotion system, based on Unreal’s Game Animation Sample. This plugin simplifies the process of integrating motion matching and other advanced animation features into your project, using a set of easy-to-use Blueprint or C++ functions.

### Key Features:
- **Easy Setup:** Integrate a full locomotion system with minimal effort.
- **Flexible Integration:** Add the system to any project, whether you're using Blueprint or C++.
- **Motion Matching:** Built upon Unreal's Game Animation Sample, bringing advanced animation techniques to your project.

### Disclaimer:
I originally was planning on integrating the Motion Matching system into my project. I decided to turn it into a plugin to share with the community. Please note, I am an intermediate programmer, so any feedback or improvements are welcome!

### Contribution:
If you have any suggestions, improvements, or run into any issues, feel free to share them. Your input will help make this plugin better.

Hope you find this plugin helpful!


## How to set up de plugin.

1. **Clone and Install the Plugin**

   - Navigate to your project's root folder. If a "Plugins" folder doesn't already exist, create one.

     ![image](https://github.com/user-attachments/assets/00b7b411-cb63-4ee0-b30c-bec46b04ad60)

   - Drag and drop the downloaded plugin into the "Plugins" folder.

     ![image](https://github.com/user-attachments/assets/5d7a2d0e-6948-4a4b-948b-1f1dc70c497d)

   - Launch your project. When prompted to rebuild the project, click "Yes" to continue.

Here’s an improved version of the second setup instruction:

2. **Adding the Traversal Actor Component**

   - Open your character's Blueprint. If you're using the third-person template, this will likely be "BP_ThirdPerson."

   - In the Components tab, click **Add** and search for `MMAS_ACTraversal`.

     ![image](https://github.com/user-attachments/assets/0be81554-2466-4c2e-9ff3-100be1d6378d)

   - In the Details panel, under the **EANNAMMAS | Traversal** section, locate **TraversalMontagesChooserTable** and assign it the UAsset `CHT_Traversal`.

     ![image](https://github.com/user-attachments/assets/f8ee2710-7bc7-4422-b137-84f779a5c3bd)

   - If you don't see the asset, make sure **Show Plugin Content** is enabled.

     ![image](https://github.com/user-attachments/assets/d5f4174d-b9cc-4f1d-b3e1-4fc5811b1db9)

   - Additionally, the **Debug** category is available for monitoring and debugging traversal-related variables.
3. 


   
