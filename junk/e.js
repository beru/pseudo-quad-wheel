try {
    throw "aaa";
} catch (e) {
    console.log(typeof e);
    console.log(e);
}