/*
=!EXPECTSTART!=
ZhangSan
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
	   
	function Employee(name, sex, employeeID) {
	    this.name = name;
	    this.sex = sex;
	    this.employeeID = employeeID;
	};
	
	// ��Employee��ԭ��ָ��Person��һ��ʵ��
	// ��ΪPerson��ʵ�����Ե���Personԭ���еķ���, ����Employee��ʵ��Ҳ���Ե���Personԭ���е��������ԡ�
	Employee.prototype = new Person();
	Employee.prototype.getEmployeeID = function() {
	    return this.employeeID;
	};
	var zhang = new Employee("ZhangSan", "man", "1234");
	console.log(zhang.getName()); // "ZhangSan
