/*
=!EXPECTSTART!=
ZhangSan
ChunHua
18
20
18
=!EXPECTEND!=
*/

	// ���캯��
	   function Person(name, sex) {
	       this.name = name;
	       this.sex = sex;
	   };
	   // ����Person��ԭ�ͣ�ԭ���е����Կ��Ա��Զ����������
	   Person.prototype = {
	       getName: function() {
	           return this.name;
	       },
	       getSex: function() {
	           return this.sex;
	       },
		   age: 18
	   };
	var zhang = new Person("ZhangSan", "man");
	console.log(zhang.getName()); // "ZhangSan"
	var chun = new Person("ChunHua", "woman");
	console.log(chun.getName()); // "ChunHua"
	
	console.log(zhang.age);		//18
	zhang.age = 20;
	console.log(zhang.age);		//20
	delete zhang.age;
	console.log(zhang.age);
