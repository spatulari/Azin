# How to Install Azin
---

### Browser (Recommended)

1. Go to the [GitHub repository](https://github.com/Alex12-Git200/Azin)

   ![GitHub page](assets/install-tutorial/img1.png)

2. Click **Releases**

   ![Releases button](assets/install-tutorial/img2.png)

3. Download the latest **`azc.exe`** or **`azc`** file.

---

### Setup

1. Move the downloaded **`azc`** file into your project folder.

2. Create a file with the `.az` extension and write some Azin code.

Example:

```az
int add(int a, int b)
{
    return a + b;
}

int main()
{
    int result = add(5, 3);
    return result;
}
```

---

### Dependencies (Linux / WSL)

Before running Azin, install the required dependencies  
(**Debian / Ubuntu**):

```bash
sudo apt install g++ gcc nasm
```

## VSCode extension

To download the VSCode suppport extension, go to its [page](https://marketplace.visualstudio.com/items?itemName=Alex12-Git200.azin-language-support) or just search 'Azin' in VSCode extensions and it will be the top result
